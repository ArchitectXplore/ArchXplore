#include "cpu/abstractCPU.hpp"
#include "system/abstractSystem.hpp"

namespace archXplore
{
    namespace cpu
    {
        abstractCPU::abstractCPU(sparta::TreeNode *tn, const hartId_t &tid, const sparta::Clock::Frequency &freq)
            : Unit(tn), m_tid(tid), m_freq(freq), m_status(cpuStatus_t::INACTIVE),
              m_cycle(tn, "cycle", "Number of cycles elapsed", sparta::Counter::CounterBehavior::COUNT_NORMAL),
              m_instret(tn, "instret", "Number of retired instructions", sparta::Counter::CounterBehavior::COUNT_NORMAL)
        {
            getSystemPtr()->addCPU(this, tid, freq);
        };

        abstractCPU::~abstractCPU(){};

        auto abstractCPU::getSystemPtr() -> system::abstractSystem *
        {
            sparta_assert((system::g_system_ptr != nullptr), "System was not instanted!");
            return system::g_system_ptr;
        };

    } // namespace cpu

} // namespace archXplore
