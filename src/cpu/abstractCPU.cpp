#include "cpu/abstractCPU.hpp"
#include "system/abstractSystem.hpp"

namespace archXplore
{
    namespace cpu
    {
        abstractCPU::abstractCPU(sparta::TreeNode *tn, const iss::hartId_t& tid, const sparta::Clock::Frequency &freq)
            : m_tid(tid), m_freq(freq),
              Unit(tn)
        {
            getSystemPtr()->addCPU(this, tid, freq);
        };

        abstractCPU::~abstractCPU(){
        };


        auto abstractCPU::getSystemPtr() -> system::abstractSystem *
        {
            sparta_assert((system::g_system_ptr != nullptr), "System was not instanted!");
            return system::g_system_ptr;
        };

    } // namespace cpu

} // namespace archXplore
