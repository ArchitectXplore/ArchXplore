#pragma once

#include <atomic>

#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/statistics/Counter.hpp"

#include "iss/abstractISS.hpp"

namespace archXplore
{

    // Forward Declaration
    namespace system
    {
        class abstractSystem;
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

        class abstractCPU : public sparta::Unit
        {
        public:

            // Delete copy-construct function
            abstractCPU(const abstractCPU &that) = delete;
            abstractCPU &operator=(const abstractCPU &that) = delete;

            /**
             * @brief Constructor for the abstractCPU class
             *
             * @param tn The TreeNode where this CPU is being instantiated.
             */
            abstractCPU(sparta::TreeNode *tn);

            /**
             * @brief Destructor for the abstractCPU class
             */
            ~abstractCPU();

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
            inline auto getBootPC() -> const addr_t&;

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
             * @brief Set up the wakeup monitor
             *
             * This function is called to set up the wakeup monitor.
             **/
            auto setUpWakeUpMonitor() -> void;

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
            inline auto getSystemPtr() -> system::abstractSystem *;

            /**
             * @brief Get the thread ID
             *
             * This function is called to get the thread ID.
             * @return The thread ID.
             */
            auto getThreadID() -> const hartId_t;

            /**
             * @brief Set the instruction set simulator pointer
             *
             * This function is called to set the instruction set simulator pointer.
             * @param iss The instruction set simulator pointer.
             */
            auto setISS(std::unique_ptr<iss::abstractISS> iss) -> void;

        public:
            // CPU Status
            cpuStatus_t m_status;
            // Boot Address
            addr_t m_boot_pc;
            // Cycle counter
            sparta::Counter m_cycle;
            // Instruction retired counter
            sparta::Counter m_instret;
            // Unique Thread Id
            hartId_t m_tid;
            // Processor frequency
            sparta::Clock::Frequency m_freq;

        protected:
            // ISS Ptr
            std::unique_ptr<iss::abstractISS> m_iss;
            // Wakeup monitor event
            sparta::UniqueEvent<sparta::SchedulingPhase::Update> m_wakeup_monitor_event;
            // Tick event
            sparta::UniqueEvent<sparta::SchedulingPhase::Tick> m_tick_event;
        };

    } // namespace iss
} // namespace archXplore
