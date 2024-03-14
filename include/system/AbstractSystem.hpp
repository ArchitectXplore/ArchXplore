#pragma once

#include <unordered_map>
#include <mutex>
#include <map>
#include <future>
#include <list>
#include <csignal>

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"

#include "ClockedObject.hpp"

#include "utils/ThreadPool.hpp"
#include "cpu/AbstractCPU.hpp"
#include "iss/AbstractISS.hpp"

#include "system/Process.hpp"


namespace archXplore
{

    // Forward declaration
    namespace cpu
    {
        class AbstractCPU;
    }

    namespace system
    {

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

        class AbstractSystem : public ClockedObject
        {
        public:

            static constexpr const char *INFO_LOG = "info";
            static constexpr const char *WARN_LOG = sparta::log::categories::WARN_STR;
            static constexpr const char *DEBUG_LOG = sparta::log::categories::DEBUG_STR;

            /**
             * @brief Boot the system
             */
            virtual auto bootSystem() -> void = 0;

            /**
             * @brief Create the instruction set simulator
             * @return Pointer to the instruction set simulator object
             */
            virtual auto createISS() -> std::unique_ptr<iss::AbstractISS> = 0;

            /**
             * @brief Clean up the system
             */
            virtual auto cleanUp() -> void{};

            // Delete Copy function
            AbstractSystem(const AbstractSystem &that) = delete;
            AbstractSystem &operator=(const AbstractSystem &that) = delete;

            /**
             * @brief Default Constructor which sets the clock frequency to 1GHz
             */
            AbstractSystem();

            /**
             * @brief Constructor
             * @param freq The clock frequency of the system
             */
            AbstractSystem(const sparta::Clock::Frequency &freq);

            /**
             * @brief Destructor
             */
            ~AbstractSystem();

            /**
             * @brief Get the CPU count
             * @return The number of CPUs in the system
             */
            auto getCPUCount() -> uint32_t;

            /**
             * @brief Get the executable path of the running binary
             * @return The executable path
             */
            auto executablePath() const -> std::string;

            /**
             * @brief Get the application name
             * @return The application name
             */
            auto getAppName() const -> std::string;

            /**
             * @brief New process
             * @param process Pointer to the process object
             * @return Pointer to the process object
             */
            auto newProcess(Process *process) -> Process *;

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
             * @brief Get the elapsed time of the system
             * @return The elapsed time in seconds
             */
            auto getElapsedTime() const -> double;

            /**
             * @brief Register the instruction set simulator to the system
             */
            auto registerISS() -> void;

            /**
             * @brief Get the CPU pointer by hart id
             * @param tid Hart id
             * @return Pointer to the CPU object
             */
            auto getCPUPtr(const HartID_t &tid) -> cpu::AbstractCPU *;

            /**
             * @brief Get the system pointer
             * @return Pointer to the system object
             */
            static inline auto getSystemPtr() -> AbstractSystem *
            {
                sparta_assert((m_system_ptr != nullptr), "Can't get system pointer before build it\n");
                return m_system_ptr;
            };

            /**
             * @brief Register a CPU to the system
             * @param cpu Pointer to the CPU object
             */
            virtual auto registerCPU(cpu::AbstractCPU *cpu) -> void;

            /**
             * @brief Register a clock domain to the system
             * @param freq Clock frequency
             * @param nodes Nodes that belong to the clock domain
             */
            auto registerClockDomain(sparta::TreeNode *node, const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void;

        public:
            // System parameters
            bool m_multithread_enabled = false;

            sparta::Scheduler::Tick m_tick_limit;
            uint64_t m_multithread_interval;

            // Maximum interval for multi-threading simulation
            const uint64_t MAX_INTERVAL = sparta::Scheduler::INDEFINITE;

            // Workload parameters
            std::string m_workload_path;
            std::list<std::string> m_workload_args;
            // Workloads
            std::vector<Process *> m_processes;

            std::vector<cpu::AbstractCPU *> m_cpus;
            const sparta::Clock::Frequency m_system_freq;

        protected:
            // Thread pool for multi-threading simulation
            std::unique_ptr<utils::ThreadPool> m_thread_pool;
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

            //! Default info logger
            sparta::log::MessageSource m_info_logger;

            //! Default warn logger
            sparta::log::MessageSource m_warn_logger;

            //! Default debug logger
            sparta::log::MessageSource m_debug_logger;

            // Root TreeNode
            sparta::RootTreeNode m_root_node;

            // System ptr
            static AbstractSystem *m_system_ptr;
        };

        class PseudoSystem : public AbstractSystem
        {
        public:
            PseudoSystem();
            ~PseudoSystem();
            auto bootSystem() -> void override;
            auto createISS() -> std::unique_ptr<iss::AbstractISS> override;
            auto registerCPU(cpu::AbstractCPU *cpu) -> void override;
        };
        

    } // namespace system
} // namespace archXplore
