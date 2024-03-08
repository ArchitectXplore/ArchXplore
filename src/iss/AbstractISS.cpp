#include "iss/AbstractISS.hpp"

namespace archXplore
{
    namespace iss
    {

        auto AbstractISS::setCPU(cpu::AbstractCPU *cpu) -> void
        {
            m_cpu = cpu;
            this->initialize();
        };

        AbstractISS::AbstractISS() = default;

        AbstractISS::~AbstractISS() = default;
    }
}