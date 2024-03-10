#pragma once

#include <atomic>

#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/statistics/Counter.hpp"

#include "system/Process.hpp"

#include "iss/AbstractISS.hpp"

namespace archXplore
{

    // Forward Declaration
    namespace system
    {
        class AbstractSystem;
    }

    namespace cpu
    {

        enum cpuStatus_t
        {
            INACTIVE,
            ACTIVE,
            BLOCKED_SYSCALL,
            BLOCKED_COMM,
            BLOCKED_MUTEX,
            BLOCKED_BARRIER,
            BLOCKED_COND,
            BLOCKED_JOIN,
            COMPLETED,
            NUM_STATUSES
        };

        class AbstractCPU : public sparta::Unit
        {
        public:

            // Delete copy-construct function
            AbstractCPU(const AbstractCPU &that) = delete;
            AbstractCPU &operator=(const AbstractCPU &that) = delete;

            /**
             * @brief Constructor for the AbstractCPU class
             *
             * @param tn The TreeNode where this CPU is being instantiated.
             */
            AbstractCPU(sparta::TreeNode *tn);

            /**
             * @brief Destructor for the AbstractCPU class
             */
            ~AbstractCPU();

            /**
             * @brief Reset the CPU
             *
             * This function is called to reset the CPU.
             **/
            virtual auto reset() -> void = 0;

            /**
             * @brief Tick the CPU
             *
             * This function is called to tick the CPU.
             **/
            inline virtual auto tick() -> void = 0;

            /**
             * @brief Get the boot PC
             *
             * This function is called to get the boot PC.
             * @return The boot PC.
             */
            inline auto getBootPC() -> const Addr_t&;

            /**
             * @brief Start up the CPU
             *
             * This function is called to start up the CPU.
             **/
            auto startUp() -> void;

            /**
             * @brief Handle a tick event
             *
             * This function is called to handle a tick event.
             **/
            auto handleTickEvent() -> void;

            /**
             * @brief Schedule the startup event
             *
             * This function is called to schedule the startup event.
             **/
            auto scheduleStartupEvent() -> void;

            /**
             * @brief Schedule the wakeup monitor event
             *
             * This function is called to schedule the wakeup monitor event.
             **/
            auto scheduleWakeUpMonitorEvent() -> void;

            /**
             * @brief Cancel the wakeup monitor event
             *
             * This function is called to cancel the wakeup monitor event.
             **/
            auto cancelWakeUpMonitorEvent() -> void;

            /**
             * @brief Schedule the next tick event
             *
             * This function is called to schedule the next tick event.
             **/
            auto scheduleNextTickEvent() -> void;

            /**
             * @brief Cancel the next tick event
             *
             * This function is called to cancel the next tick event.
             **/
            auto cancelNextTickEvent() -> void;

            /**
             * @brief Handle the wakeup monitor
             *
             * This function is called to handle the wakeup monitor.
             **/
            auto handleWakeUpMonitorEvent() -> void;

            /**
             * @brief Indicates if the CPU is running
             *
             * This function is called to indicate if the CPU is running.
             * @return true if the CPU is running, false otherwise.
             */
            inline auto isRunning() const -> bool;

            /**
             * @brief Indicates if the CPU is blocked
             *
             * This function is called to indicate if the CPU is blocked.
             * @return true if the CPU is blocked, false otherwise.
             */
            inline auto isBlocked() const -> bool;

            /**
             * @brief Indicates if the CPU is completed
             *
             * This function is called to indicate if the CPU is completed.
             * @return true if the CPU is completed, false otherwise.
             */
            inline auto isCompleted() const -> bool;

            /**
             * @brief Get the system pointer
             *
             * This function is called to get the system pointer.
             * @return The system pointer.
             */
            auto getSystemPtr() -> system::AbstractSystem *;

            /**
             * @brief Get the hart ID
             *
             * This function is called to get the thread ID.
             * @return The thread ID.
             */
            auto getHartID() -> const HartID_t;

            /**
             * @brief Set the instruction set simulator pointer
             *
             * This function is called to set the instruction set simulator pointer.
             * @param iss The instruction set simulator pointer.
             */
            auto setISS(std::unique_ptr<iss::AbstractISS> iss) -> void;

            /**
             * @brief Set the process pointer
             *
             * This function is called to set the process pointer.
             * @param process The process pointer.
             */
            auto setProcess(system::Process *process) -> void;

        public:
            // CPU Status
            cpuStatus_t m_status;
            // Boot Address
            Addr_t m_boot_pc;
            // Cycle counter
            sparta::Counter m_cycle;
            // Instruction retired counter
            sparta::Counter m_instret;
            // Unique Hart Id
            HartID_t m_hart_id;
            // Processor frequency
            sparta::Clock::Frequency m_freq;
            // Logger for tracing
            sparta::log::MessageSource m_trace_logger;
            // Process pointer
            system::Process *m_process;

        protected:
            // ISS Ptr
            std::unique_ptr<iss::AbstractISS> m_iss;
            // Wakeup monitor event
            sparta::UniqueEvent<sparta::SchedulingPhase::Update> m_wakeup_monitor_event;
            // Tick event
            sparta::UniqueEvent<sparta::SchedulingPhase::Tick> m_tick_event;
            // Startup event
            sparta::UniqueEvent<sparta::SchedulingPhase::Tick> m_startup_event;
        };

    } // namespace iss
} // namespace archXplore
