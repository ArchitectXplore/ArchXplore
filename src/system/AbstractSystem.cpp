#include "system/AbstractSystem.hpp"

namespace archXplore
{
    namespace system
    {
        AbstractSystem *g_system_ptr = new PseudoSystem();

        AbstractSystem::AbstractSystem() : AbstractSystem(sparta::Clock::Frequency(1000)){};

        AbstractSystem::AbstractSystem(const sparta::Clock::Frequency &freq)
            : RootTreeNode("System"), m_global_event_set(this),
              m_system_freq(freq), m_multithread_interval(1000 * freq)
        {
            g_system_ptr = this;
            // Create clock domains 0 as global clock domain
            this->registerClockDomain(this, 0, freq);
        };

        AbstractSystem::~AbstractSystem(){};

        auto AbstractSystem::getCPUCount() -> uint32_t
        {
            return m_cpus.size();
        };

        auto AbstractSystem::executablePath() -> const std::string
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
        };

        auto AbstractSystem::getAppName() -> const std::string
        {
            auto pid = getpid();
            return "ArchXplore_" + std::to_string(pid);
        };

        auto AbstractSystem::newProcess(Process *process) -> Process *
        {
            process->pid = m_processes.size();
            m_processes.emplace_back(process);
            return process;
        };

        auto AbstractSystem::build() -> void
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

        auto AbstractSystem::finalize() -> void
        {
            // Bind tree early
            bindTreeEarly();
            // Finalize scheduler
            for (auto &it : m_rank_domains)
            {
                auto &rank_domain = it.second;
                rank_domain.scheduler->finalize();
            }
            // Construct thread pool when multi-threading is enabled
            if (m_multithread_enabled)
            {
                std::size_t num_threads = std::min(m_cpus.size(), m_rank_domains.size() - 1);
                m_thread_pool = std::make_unique<utils::ThreadPool>(num_threads);
            }
            // Bind tree late
            bindTreeLate();
            // Enter teardown state
            enterTeardown();
            // Boot the system and enter teardown state
            bootSystem();
        };

        auto AbstractSystem::startUpTick(const sparta::Scheduler::Tick &tick) -> void
        {
            m_tick_limit = m_main_scheduler->getCurrentTick() + tick;
            if (m_multithread_enabled)
            {
                handlePreTickEvent();
            }
        };

        auto AbstractSystem::handlePreTickEvent() -> void
        {
            sparta::Scheduler::Tick tick;
            if (SPARTA_EXPECT_FALSE(m_main_scheduler->getCurrentTick() + m_multithread_interval >= m_tick_limit))
            {
                tick = m_tick_limit - m_main_scheduler->getCurrentTick();
            }
            else
            {
                tick = m_multithread_interval;
            }
            if (SPARTA_EXPECT_TRUE(tick))
            {
                for (auto &it : m_rank_domains)
                {
                    auto &rank_id = it.first;
                    auto &rank_domain = it.second;
                    auto scheduler = rank_domain.scheduler.get();
                    if (rank_id)
                    {
                        m_thread_futures.emplace_back(m_thread_pool->enqueue(
                            [=]
                            {
                                scheduler->run(tick, true, false);
                                return scheduler->isFinished();
                            }));
                    }
                }
                m_post_tick_event->scheduleRelativeTick(tick - 1, m_main_scheduler);
            }
        };

        auto AbstractSystem::handlePostTickEvent() -> void
        {
            bool continue_simulation = false;

            while (!m_thread_futures.empty())
            {
                if (!m_thread_futures.front().get())
                {
                    continue_simulation = true;
                }
                m_thread_futures.pop_front();
            }

            if (continue_simulation)
            {
                m_pre_tick_event->scheduleRelativeTick(1, m_main_scheduler);
            };
        };

        auto AbstractSystem::buildClockDomains() -> void
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
                m_pre_tick_event = std::make_unique<sparta::UniqueEvent<sparta::SchedulingPhase::Update>>(
                    &m_global_event_set, "pre_tick_event",
                    CREATE_SPARTA_HANDLER(AbstractSystem, handlePreTickEvent));
                m_post_tick_event = std::make_unique<sparta::UniqueEvent<sparta::SchedulingPhase::PostTick>>(
                    &m_global_event_set, "post_tick_event",
                    CREATE_SPARTA_HANDLER(AbstractSystem, handlePostTickEvent));
            }
        };

        auto AbstractSystem::run(sparta::Scheduler::Tick tick) -> void
        {
            if (SPARTA_EXPECT_FALSE(!m_finalized))
            {
                finalize();
            }
            startUpTick(tick);
            m_main_scheduler->run(tick, true, false);
        }

        auto AbstractSystem::registerISS() -> void
        {
            for (auto &cpu : m_cpus)
            {
                cpu->setISS(_createISS());
            }
        };

        auto AbstractSystem::getCPUPtr(const HartID_t &tid) -> cpu::AbstractCPU *
        {
            sparta_assert(tid < m_cpus.size(), "Invalid hart id\n");
            return m_cpus[tid];
        };

        auto AbstractSystem::registerCPU(cpu::AbstractCPU *cpu) -> void
        {
            sparta_assert(cpu != nullptr, "CPU pointer is null\n");
            const HartID_t hart_id = m_cpus.size();
            cpu->m_hart_id = hart_id;
            cpu->m_freq = cpu->getClock()->getFrequencyMhz();
            m_cpus.push_back(cpu);
        };

        auto AbstractSystem::registerClockDomain(TreeNode *node, const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void
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

        // PseudoSystem implementation
        PseudoSystem::PseudoSystem() = default;
        PseudoSystem::~PseudoSystem() = default;
        auto PseudoSystem::bootSystem() -> void
        {
            sparta_throw("Can't boot pseudo system!");
        };
        auto PseudoSystem::_createISS() -> std::unique_ptr<iss::AbstractISS>
        {
            sparta_throw("Can't create iss within pseudo system!");
            return nullptr;
        };
        auto PseudoSystem::registerCPU(cpu::AbstractCPU *cpu) -> void
        {};

    } // namespace system

} // namespace archXplore
