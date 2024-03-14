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
#include "sparta/events/StartupEvent.hpp"

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

        struct ClockDomain_t
        {
            sparta::Clock::Handle clock;
            std::vector<sparta::TreeNode *> nodes;
        };

        struct RankDomain_t
        {
            std::string name;
            // Rank scheduler and clock used by sub-thread
            sparta::Clock::Handle clock;
            std::unique_ptr<sparta::Scheduler> scheduler;
            std::unique_ptr<sparta::ClockManager> clock_manager;
            // Tree nodes that belong to this clock domain
            std::map<sparta::Clock::Frequency, ClockDomain_t> domains;
        };

        typedef std::map<uint32_t, RankDomain_t> PhaseDomain_t;

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
             * @brief Run phase event
             * @param phase Schedule phase
             * @return True if the phase is done, false otherwise
             */
            auto runPhaseEvent(PhaseDomain_t& phase) -> bool;

            /**
             * @brief Handle the bound-weave event
             */
            auto handleBoundWeaveEvent() -> void;


            /**
             * @brief Build the rank domains of the system
             */
            auto buildRank(RankDomain_t& rank) -> void;

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
             * @param nodes Nodes that belong to the clock domain
             * @param phase Schedule phase of the clock domain
             * @param rank Rank of the clock domain
             * @param freq Clock frequency of the clock domain
             */
            auto registerClockDomain(TreeNode *node, const SchedulePhase_t &phase,
                                     const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void;

        public:
            // System parameters
            uint64_t m_max_threads = 0;
            bool m_bound_weave_enabled = false;
            uint64_t m_bound_weave_interval = 1e6; // 1us

            // Workloads
            std::vector<Process *> m_processes;

            std::vector<cpu::AbstractCPU *> m_cpus;
            const sparta::Clock::Frequency m_system_freq;

        protected:
            // Thread pool for multi-threading simulation
            std::unique_ptr<utils::ThreadPool> m_thread_pool;

            RankDomain_t m_schedule_phase;

            // Bound and weave phases domain
            PhaseDomain_t m_bound_phase;
            PhaseDomain_t m_weave_phase;

            // Main scheduler
            sparta::Scheduler *m_main_scheduler;
            sparta::Clock::Handle m_global_clock;

            // Global Event Set
            sparta::EventSet m_global_event_set;

            // Bound-Weave event
            sparta::UniqueEvent<sparta::SchedulingPhase::Tick> m_bound_weave_event;

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
