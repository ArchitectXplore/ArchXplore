#pragma once

#include <unordered_map>
#include <mutex>
#include <map>
#include <future>

#include "sparta/sparta.hpp"
#include "sparta/simulation/ClockManager.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/events/PayloadEvent.hpp"

#include "cpu/abstractCPU.hpp"
#include "iss/abstractISS.hpp"
#include "utils/threadPool.hpp"

namespace archXplore
{

    namespace system
    {
        class abstractSystem;

        extern abstractSystem *g_system_ptr;

        struct clockDomain_t
        {
            sparta::Clock::Handle clock;
            std::vector<sparta::TreeNode *> nodes;
        };

        struct rankDomain_t
        {
            // Rank scheduler and clock used by sub-thread
            sparta::Clock::Handle clock;
            std::unique_ptr<sparta::Scheduler> scheduler;
            std::unique_ptr<sparta::ClockManager> clock_manager;
            // Tree nodes that belong to this clock domain
            std::map<sparta::Clock::Frequency, clockDomain_t> domains;
        };

        class abstractSystem : public sparta::RootTreeNode
        {
        private:
            struct processorInfo_t
            {
                sparta::Clock::Frequency freq;
                cpu::abstractCPU *cpu;
            };
            using processorInfoMap_t = std::unordered_map<hartId_t, processorInfo_t>;

        public:
            // Delete Copy function
            abstractSystem(const abstractSystem &that) = delete;
            abstractSystem &operator=(const abstractSystem &that) = delete;

            /**
             * @brief Default Constructor which sets the clock frequency to 1GHz
             */
            abstractSystem() : abstractSystem(sparta::Clock::Frequency(1000)){};

            /**
             * @brief Constructor
             * @param freq The clock frequency of the system
             */
            abstractSystem(const sparta::Clock::Frequency &freq)
                : RootTreeNode("System"), m_global_event_set(this),
                  m_system_freq(freq), m_multithread_interval(1000 * freq)
            {
                g_system_ptr = this;
                // Create clock domains 0 as global clock domain
                this->registerClockDomain(this, 0, freq);
            };

            ~abstractSystem(){};

            /**
             * @brief Get the CPU count
             * @return The number of CPUs in the system
             */
            virtual auto getCPUCount() -> uint32_t
            {
                return m_cpus.size();
            };

            virtual auto bootSystem() -> void{};

            /**
             * @brief Create the instruction set simulator
             * @return Pointer to the instruction set simulator object
             */
            virtual auto _createISS() -> iss::abstractISS::UniquePtr = 0;

            /**
             * @brief Get the executable path of the running binary
             * @return The executable path
             */
            auto executablePath() -> const std::string
            {
                char buffer[4096];
                std::string binary_path;
                std::string binary_dir;
                ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
                if (length != -1)
                {
                    buffer[length] = '\0';
                    binary_path = buffer;
                    size_t lastSlash = binary_path.find_last_of("/\\");
                    binary_dir = binary_path.substr(0, lastSlash);
                }
                else
                {
                    std::cerr << "Can't get binary file location." << std::endl;
                    std::abort();
                }
                return std::string(binary_dir);
            }

            /**
             * @brief Build the system
             */
            auto build() -> void
            {
                // Enter configuring state
                enterConfiguring();
                // Build clock domains and rank schedulers
                buildClockDomains();
                // Finalize tree and create resources
                enterFinalized();
                // Register instruction set simulator
                registerISS();
            };

            /**
             * @brief Finalize the system
             */
            auto finalize() -> void
            {
                // Finalize scheduler
                for (auto &it : m_rank_domains)
                {
                    auto &rank_domain = it.second;
                    rank_domain.scheduler->finalize();
                }
                // Construct thread pool when multi-threading is enabled
                if (m_multithread_enabled)
                {
                    m_thread_pool = std::make_unique<utils::threadPool>(m_cpus.size());
                }
                // Boot the system and enter teardown state
                bootSystem();
                enterTeardown();
            };

            /**
             * @brief Handle the start of multi-threaded simulation
             * @param rank_id The rank id to start
             * @return True if the simulation is finished, false otherwise
             */
            auto handleTickEventStart(uint32_t rank_id) -> bool
            {
                if(rank_id == 4) {
                    std::cout << "Thread " << rank_id << " started" << std::endl;
                }
                sparta_assert(m_multithread_enabled, "Multi-threading is not enabled\n");
                sparta_assert(m_rank_domains.find(rank_id) != m_rank_domains.end(),
                              "Invalid rank id " + std::to_string(rank_id) + "\n");

                auto &scheduler = m_rank_domains[rank_id].scheduler;
                auto &clock = m_rank_domains[rank_id].clock;
                scheduler->run(this->m_multithread_interval, true, false);
                return scheduler->isFinished();
            };

            /**
             * @brief Handle the end of multi-threaded simulation
             */
            auto handleTickEventEnd() -> void
            {
                sparta_assert(m_multithread_enabled, "Multi-threading is not enabled\n");

                std::map<uint32_t, std::future<bool>> new_thread_futures;

                for (auto &it : m_thread_futures)
                {
                    auto & rank_id = it.first;
                    if (!it.second.get())
                    {
                        
                        new_thread_futures[rank_id] = m_thread_pool->enqueue(
                            [this, rank_id]
                            {
                                return this->handleTickEventStart(rank_id);
                            });
                    }
                }

                if (!new_thread_futures.empty())
                {
                    m_thread_futures = std::move(new_thread_futures);
                    m_rank_tick_end_event->schedule();
                }
            };

            /**
             * @brief Handle the start up of multi-threaded simulation
             */
            auto handleMultiThreadStartUp() -> void
            {
                sparta_assert(m_multithread_enabled, "Multi-threading is not enabled\n");
                for(auto &it : m_rank_domains) 
                {
                    auto &rank_id = it.first;
                    auto &rank_domain = it.second;
                    if(rank_id) {
                        m_thread_futures[rank_id] = m_thread_pool->enqueue(
                            [this, rank_id]
                            {
                                return this->handleTickEventStart(rank_id);
                            });
                    }
                }
                m_rank_tick_end_event->schedule();
            }

            /**
             * @brief Build the clock domains of the system
             */
            auto buildClockDomains() -> void
            {
                for (auto &it : m_rank_domains)
                {
                    auto &rank_id = it.first;
                    auto &rank_domain = it.second;
                    rank_domain.scheduler = std::make_unique<sparta::Scheduler>("Scheduler_Rank_" + std::to_string(rank_id));
                    rank_domain.clock_manager = std::make_unique<sparta::ClockManager>(rank_domain.scheduler.get());
                    rank_domain.clock_manager->makeRoot(this, "Rank" + std::to_string(rank_id) + "Clock");
                    rank_domain.clock = rank_domain.clock_manager->makeClock("Rank" + std::to_string(rank_id) + "_clock",
                                                                             rank_domain.clock_manager->getRoot(), m_system_freq);
                    for (auto &domain : rank_domain.domains)
                    {
                        auto &freq = domain.first;
                        auto &clock_domain = domain.second;
                        clock_domain.clock = rank_domain.clock_manager->makeClock("clock_domain_" + std::to_string(uint64_t(freq)) + "Mhz",
                                                                                  rank_domain.clock, freq);
                        for (auto &node : clock_domain.nodes)
                        {
                            node->setClock(clock_domain.clock.get());
                        }
                    }
                    rank_domain.clock_manager->normalize();
                }
                m_main_scheduler = m_rank_domains[0].scheduler.get();
                m_global_clock = m_rank_domains[0].clock;
                // Enable multi-threaded simulation
                if (m_rank_domains.size() > 1)
                {
                    m_multithread_enabled = true;
                    m_rank_tick_end_event = std::make_unique<sparta::Event<sparta::SchedulingPhase::Update>>(
                        &m_global_event_set, "rank_tick_end_event",
                        CREATE_SPARTA_HANDLER(abstractSystem, handleTickEventEnd), m_multithread_interval);
                    // Add startup event to the scheduler
                    sparta::StartupEvent(&m_global_event_set, CREATE_SPARTA_HANDLER(abstractSystem, handleMultiThreadStartUp));
                }
            };

            /**
             * @brief Run the system
             * @param tick The tick to run the system
             */
            auto run(sparta::Scheduler::Tick tick) -> void
            {
                if (SPARTA_EXPECT_FALSE(!m_finalized))
                {
                    finalize();
                }
                m_main_scheduler->run(tick, false, false);
            }

            /**
             * @brief Register the instruction set simulator to the system
             */
            auto registerISS() -> void
            {
                for (auto &cpu : m_cpus)
                {
                    cpu->setISS(_createISS());
                }
            };

            /**
             * @brief Get the CPU pointer by hart id
             * @param tid Hart id
             * @return Pointer to the CPU object
             */
            auto getCPUPtr(const hartId_t &tid) -> cpu::abstractCPU *
            {
                sparta_assert(tid < m_cpus.size(), "Invalid hart id\n");
                return m_cpus[tid];
            };

            /**
             * @brief Get the system pointer
             * @return Pointer to the system object
             */
            static auto getSystemPtr() -> abstractSystem *
            {
                sparta_assert((g_system_ptr != nullptr), "Can't get system pointer before build it\n");
                return g_system_ptr;
            };

            /**
             * @brief Register a CPU to the system
             * @param cpu Pointer to the CPU object
             */
            virtual auto registerCPU(cpu::abstractCPU *cpu) -> void
            {
                sparta_assert(cpu != nullptr, "CPU pointer is null\n");
                const hartId_t tid = m_cpus.size();
                cpu->m_tid = tid;
                m_cpus.push_back(cpu);
            };

            /**
             * @brief Register a clock domain to the system
             * @param freq Clock frequency
             * @param nodes Nodes that belong to the clock domain
             */
            auto registerClockDomain(TreeNode *node, const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void
            {
                if (m_rank_domains.find(rank) == m_rank_domains.end())
                {
                    m_rank_domains[rank] = rankDomain_t();
                }
                auto &clock_domains = m_rank_domains[rank].domains;
                if (clock_domains.find(freq) == clock_domains.end())
                {
                    clock_domains[freq] = clockDomain_t();
                }
                clock_domains[freq].nodes.push_back(node);
            };

        public:
            // System parameters
            bool m_multithread_enabled = false;
            uint64_t m_multithread_interval;

            std::string m_workload_path;
            std::vector<cpu::abstractCPU *> m_cpus;
            const sparta::Clock::Frequency m_system_freq;

        protected:
            // Thread pool for multi-threading simulation
            std::unique_ptr<utils::threadPool> m_thread_pool;
            std::map<uint32_t, std::future<bool>> m_thread_futures;
            // Rank & Clock domains
            std::map<uint32_t, rankDomain_t> m_rank_domains;
            // Main scheduler
            sparta::Scheduler *m_main_scheduler;
            sparta::Clock::Handle m_global_clock;
            // Rank tick end event
            std::unique_ptr<sparta::Event<sparta::SchedulingPhase::Update>> m_rank_tick_end_event;
            // Global Event Set
            sparta::EventSet m_global_event_set;
            // finalized flag
            bool m_finalized = false;
        };

    } // namespace system
} // namespace archXplore
