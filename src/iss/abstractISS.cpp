#include "iss/abstractISS.hpp"

namespace archXplore
{
    namespace iss
    {

        auto abstractISS::setCPU(cpu::abstractCPU *cpu) -> void
        {
            m_cpu = cpu;
            this->initialize();
        };

        abstractISS::abstractISS() = default;

        abstractISS::~abstractISS() = default;
    }
}