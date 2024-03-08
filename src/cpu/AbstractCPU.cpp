#include "sparta/events/StartupEvent.hpp"

#include "cpu/AbstractCPU.hpp"
#include "system/AbstractSystem.hpp"

namespace archXplore
{
    namespace cpu
    {
        AbstractCPU::AbstractCPU(sparta::TreeNode *tn)
            : Unit(tn), m_status(cpuStatus_t::INACTIVE),
              m_cycle(this->getStatisticSet(), "totalCycle", "Number of cycles elapsed", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_instret(this->getStatisticSet(), "totalInstRetired", "Number of retired instructions", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_wakeup_monitor_event(this->getEventSet(), "wakeUpMonitor",
                                     CREATE_SPARTA_HANDLER(AbstractCPU, handleWakeUpMonitorEvent), sparta::Clock::Cycle(1)),
              m_tick_event(this->getEventSet(), "tickEvent",
                           CREATE_SPARTA_HANDLER(AbstractCPU, handleTickEvent), sparta::Clock::Cycle(1))
        {
            getSystemPtr()->registerCPU(this);
        };

        AbstractCPU::~AbstractCPU(){};

        auto AbstractCPU::getBootPC() -> const Addr_t&
        {
            return m_boot_pc;
        };

        auto AbstractCPU::startUp() -> void
        {
            // 1. Initialize the CPU state
            m_iss->initCPUState();
            // 2. Reset the cpu
            reset();
            // 3. Schedule the first tick
            m_tick_event.schedule(sparta::Clock::Cycle(1));
        };

        auto AbstractCPU::handleTickEvent() -> void
        {
            // 1. Schedule the next tick
            scheduleNextTickEvent();
            // 2. Update the cycle counter
            m_cycle++;
            // 3. Tick the cpu
            tick();
        };

        auto AbstractCPU::handleWakeUpMonitorEvent() -> void
        {
            // 1. Schedule the next wake up monitor event
            scheduleWakeUpMonitorEvent();
            // 2. Check if the cpu is ready to wakeup
            m_iss->wakeUpMonitor();
        };

        auto AbstractCPU::scheduleWakeUpMonitorEvent() -> void 
        {
            m_wakeup_monitor_event.schedule();
        };

        auto AbstractCPU::cancelWakeUpMonitorEvent() -> void
        {
            if(SPARTA_EXPECT_TRUE(m_wakeup_monitor_event.isScheduled()))
            {
                m_wakeup_monitor_event.cancel();
            }
        };

        auto AbstractCPU::scheduleNextTickEvent() -> void
        {
            m_tick_event.schedule();
        };

        auto AbstractCPU::cancelNextTickEvent() -> void
        {
            if (SPARTA_EXPECT_TRUE(m_tick_event.isScheduled()))
            {
                m_tick_event.cancel();
            }
        };

        auto AbstractCPU::isRunning() const -> bool
        {
            return m_status > cpuStatus_t::INACTIVE && m_status < cpuStatus_t::COMPLETED;
        };
        auto AbstractCPU::isBlocked() const -> bool
        {
            return m_status > cpuStatus_t::ACTIVE && m_status < cpuStatus_t::COMPLETED;
        };
        auto AbstractCPU::isCompleted() const -> bool
        {
            return m_status == cpuStatus_t::COMPLETED;
        };

        auto AbstractCPU::getSystemPtr() -> system::AbstractSystem *
        {
            sparta_assert((system::g_system_ptr != nullptr), "System was not instantiated!");
            return system::g_system_ptr;
        };

        auto AbstractCPU::getHartID() -> const HartID_t
        {
            return m_hart_id;
        };
        auto AbstractCPU::setISS(std::unique_ptr<iss::AbstractISS> iss) -> void
        {
            sparta_assert((iss != nullptr), "Setting iss to nullptr");
            iss->setCPU(this);
            m_iss = std::move(iss);
        };

    } // namespace cpu

} // namespace archXplore
