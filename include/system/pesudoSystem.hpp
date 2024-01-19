#pragma once

#include "system/abstractSystem.hpp"
#include "sparta/utils/SpartaAssert.hpp"

namespace archXplore
{

    namespace system
    {
        /* pesudoSystem is designed for pybind11-stub generation and should never be used in real application */
        class pesudoSystem : public abstractSystem
        {
        public:
            pesudoSystem(){};
            ~pesudoSystem(){};
            auto _build() -> void override
            {
                sparta_throw("Can't build pesudo system!");
            };
            auto _run(sparta::Scheduler::Tick tick) -> void override{};
            auto _createISS() -> iss::abstractISS::UniquePtr override
            {
                return nullptr;
            };
            auto addCPU(cpu::abstractCPU *cpu, const iss::hartId_t &tid,
                        const sparta::Clock::Frequency &freq) -> void override{};
        };

    } // namespace system
} // namespace archXplore
