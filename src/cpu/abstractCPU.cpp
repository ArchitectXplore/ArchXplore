#include "cpu/abstractCPU.hpp"
#include "system/abstractSystem.hpp"
#include "sparta/events/StartupEvent.hpp"

namespace archXplore
{
    namespace cpu
    {
        abstractCPU::abstractCPU(sparta::TreeNode *tn)
            : Unit(tn), m_status(cpuStatus_t::INACTIVE),
              m_cycle(this->getStatisticSet(), "totalCycle", "Number of cycles elapsed", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_instret(this->getStatisticSet(), "totalInstRetired", "Number of retired instructions", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_wakeup_monitor_event(
                  this->getEventSet(), "wakeUpMonitor",
                  CREATE_SPARTA_HANDLER(abstractCPU, wakeUpMonitor),
                  sparta::Clock::Cycle(100))
        {
            getSystemPtr()->registerCPU(this);
            sparta::StartupEvent(this->getEventSet(),
                                 CREATE_SPARTA_HANDLER(abstractCPU, startUpMonitor));
        };

        abstractCPU::~abstractCPU(){};

        auto abstractCPU::getSystemPtr() -> system::abstractSystem *
        {
            sparta_assert((system::g_system_ptr != nullptr), "System was not instanted!");
            return system::g_system_ptr;
        };

    } // namespace cpu

} // namespace archXplore
