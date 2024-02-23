#include "iss/abstractISS.hpp"
#include "cpu/abstractCPU.hpp"

namespace archXplore
{
    namespace iss
    {
        auto abstractISS::setCPU(cpu::abstractCPU *cpu) -> void
        {
            m_cpu = cpu;
            init();
        };
    }
}