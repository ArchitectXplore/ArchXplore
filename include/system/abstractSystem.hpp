#pragma once

#include <unordered_map>
#include <mutex>
#include <map>
#include <future>
#include <list>

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"

#include "utils/threadPool.hpp"
#include "cpu/abstractCPU.hpp"
#include "iss/abstractISS.hpp"

namespace archXplore
{

    // Forward declaration
    namespace cpu
    {
        class abstractCPU;
    }

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
        public:
            /**
             * @brief Boot the system
             */
            virtual auto bootSystem() -> void = 0;

            /**
             * @brief Create the instruction set simulator
             * @return Pointer to the instruction set simulator object
             */
            virtual auto _createISS() -> std::unique_ptr<iss::abstractISS> = 0;

            // Delete Copy function
            abstractSystem(const abstractSystem &that) = delete;
            abstractSystem &operator=(const abstractSystem &that) = delete;

            /**
             * @brief Default Constructor which sets the clock frequency to 1GHz
             */
            abstractSystem();

            /**
             * @brief Constructor
             * @param freq The clock frequency of the system
             */
            abstractSystem(const sparta::Clock::Frequency &freq);

            /**
             * @brief Destructor
             */
            ~abstractSystem();

            /**
             * @brief Get the CPU count
             * @return The number of CPUs in the system
             */
            auto getCPUCount() -> uint32_t;

            /**
             * @brief Get the executable path of the running binary
             * @return The executable path
             */
            auto executablePath() -> const std::string;

            /**
             * @brief Build the system
             */
            auto build() -> void;

            /**
             * @brief Finalize the system
             */
            auto finalize() -> void;

            /**
             * @brief Start up the system
             * @param tick The tick to start the system
             */
            auto startUpTick(const sparta::Scheduler::Tick &tick) -> void;

            /**
             * @brief Handle the start of multi-threaded simulation
             */
            auto handlePreTickEvent() -> void;

            /**
             * @brief Handle the end of multi-threaded simulation
             */
            auto handlePostTickEvent() -> void;

            /**
             * @brief Build the clock domains of the system
             */
            auto buildClockDomains() -> void;

            /**
             * @brief Run the system
             * @param tick The tick to run the system
             */
            auto run(sparta::Scheduler::Tick tick) -> void;

            /**
             * @brief Register the instruction set simulator to the system
             */
            auto registerISS() -> void;

            /**
             * @brief Get the CPU pointer by hart id
             * @param tid Hart id
             * @return Pointer to the CPU object
             */
            inline auto getCPUPtr(const hartId_t &tid) -> cpu::abstractCPU *;

            /**
             * @brief Get the system pointer
             * @return Pointer to the system object
             */
            static inline auto getSystemPtr() -> abstractSystem *
            {
                sparta_assert((g_system_ptr != nullptr), "Can't get system pointer before build it\n");
                return g_system_ptr;
            };

            /**
             * @brief Register a CPU to the system
             * @param cpu Pointer to the CPU object
             */
            virtual auto registerCPU(cpu::abstractCPU *cpu) -> void;

            /**
             * @brief Register a clock domain to the system
             * @param freq Clock frequency
             * @param nodes Nodes that belong to the clock domain
             */
            auto registerClockDomain(TreeNode *node, const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void;

        public:
            // System parameters
            bool m_multithread_enabled = false;

            sparta::Scheduler::Tick m_tick_limit;
            uint64_t m_multithread_interval;

            // Workload parameters
            std::string m_workload_path;
            std::list<std::string> m_workload_args;

            std::vector<cpu::abstractCPU *> m_cpus;
            const sparta::Clock::Frequency m_system_freq;

        protected:
            // Thread pool for multi-threading simulation
            std::unique_ptr<utils::threadPool> m_thread_pool;
            std::deque<std::future<bool>> m_thread_futures;

            // Rank & Clock domains
            std::map<uint32_t, rankDomain_t> m_rank_domains;

            // Main scheduler
            sparta::Scheduler *m_main_scheduler;
            sparta::Clock::Handle m_global_clock;

            // Rank tick end event
            std::unique_ptr<sparta::UniqueEvent<sparta::SchedulingPhase::Update>> m_pre_tick_event;
            std::unique_ptr<sparta::UniqueEvent<sparta::SchedulingPhase::PostTick>> m_post_tick_event;

            // Global Event Set
            sparta::EventSet m_global_event_set;

            bool m_finalized = false;
        };

        class pseudoSystem : public abstractSystem
        {
        public:
            pseudoSystem();
            ~pseudoSystem();
            auto bootSystem() -> void override;
            auto _createISS() -> std::unique_ptr<iss::abstractISS> override;
            auto registerCPU(cpu::abstractCPU *cpu) -> void override;
        };

    } // namespace system
} // namespace archXplore
