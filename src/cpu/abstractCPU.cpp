#include "sparta/events/StartupEvent.hpp"

#include "cpu/abstractCPU.hpp"
#include "system/abstractSystem.hpp"

namespace archXplore
{
    namespace cpu
    {
        abstractCPU::abstractCPU(sparta::TreeNode *tn)
            : Unit(tn), m_status(cpuStatus_t::INACTIVE),
              m_cycle(this->getStatisticSet(), "totalCycle", "Number of cycles elapsed", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_instret(this->getStatisticSet(), "totalInstRetired", "Number of retired instructions", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_wakeup_monitor_event(this->getEventSet(), "wakeUpMonitor",
                                     CREATE_SPARTA_HANDLER(abstractCPU, handleWakeUpMonitorEvent), sparta::Clock::Cycle(1)),
              m_tick_event(this->getEventSet(), "tickEvent",
                           CREATE_SPARTA_HANDLER(abstractCPU, handleTickEvent), sparta::Clock::Cycle(1))
        {
            getSystemPtr()->registerCPU(this);
            sparta::StartupEvent(this->getEventSet(),
                                 CREATE_SPARTA_HANDLER(abstractCPU, setUpWakeUpMonitor));
        };

        abstractCPU::~abstractCPU(){};

        auto abstractCPU::getBootPC() -> const addr_t&
        {
            return m_boot_pc;
        };

        auto abstractCPU::startUp() -> void
        {
            // 1. Initialize the CPU state
            m_iss->initCPUState();
            // 2. Reset the cpu
            reset();
            // 3. Schedule the first tick
            m_tick_event.schedule(sparta::Clock::Cycle(1));
        };

        auto abstractCPU::handleTickEvent() -> void
        {
            // 1. Schedule the next tick
            scheduleNextTickEvent();
            // 2. Update the cycle counter
            m_cycle++;
            // 3. Tick the cpu
            tick();
        };

        auto abstractCPU::setUpWakeUpMonitor() -> void
        {
            // Wake up thread 0
            if (m_tid == 0)
            {
                this->startUp();
            }
            else
            {
                scheduleWakeUpMonitorEvent();
            }
        }

        auto abstractCPU::handleWakeUpMonitorEvent() -> void
        {
            // 1. Schedule the next wake up monitor event
            scheduleWakeUpMonitorEvent();
            // 2. Check if the cpu is ready to wakeup
            m_iss->wakeUpMonitor();
        };

        auto abstractCPU::scheduleWakeUpMonitorEvent() -> void 
        {
            m_wakeup_monitor_event.schedule();
        };

        auto abstractCPU::cancelWakeUpMonitorEvent() -> void
        {
            if(SPARTA_EXPECT_TRUE(m_wakeup_monitor_event.isScheduled()))
            {
                m_wakeup_monitor_event.cancel();
            }
        };

        auto abstractCPU::scheduleNextTickEvent() -> void
        {
            m_tick_event.schedule();
        };

        auto abstractCPU::cancelNextTickEvent() -> void
        {
            if (SPARTA_EXPECT_TRUE(m_tick_event.isScheduled()))
            {
                m_tick_event.cancel();
            }
        };

        auto abstractCPU::isRunning() const -> bool
        {
            return m_status > cpuStatus_t::INACTIVE && m_status < cpuStatus_t::COMPLETED;
        };
        auto abstractCPU::isBlocked() const -> bool
        {
            return m_status > cpuStatus_t::ACTIVE && m_status < cpuStatus_t::COMPLETED;
        };
        auto abstractCPU::isCompleted() const -> bool
        {
            return m_status == cpuStatus_t::COMPLETED;
        };

        auto abstractCPU::getSystemPtr() -> system::abstractSystem *
        {
            sparta_assert((system::g_system_ptr != nullptr), "System was not instantiated!");
            return system::g_system_ptr;
        };

        auto abstractCPU::getThreadID() -> const hartId_t
        {
            return m_tid;
        };
        auto abstractCPU::setISS(std::unique_ptr<iss::abstractISS> iss) -> void
        {
            sparta_assert((iss != nullptr), "Setting iss to nullptr");
            iss->setCPU(this);
            m_iss = std::move(iss);
        };

    } // namespace cpu

} // namespace archXplore
