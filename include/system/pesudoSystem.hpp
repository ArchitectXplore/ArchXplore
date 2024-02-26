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
            auto getCPUCount() -> uint32_t override
            {
                return -1;
            };
            auto bootSystem() -> void override
            {
                sparta_throw("Can't boot pseudo system!");
            };
            auto _createISS() -> iss::abstractISS::UniquePtr override
            {
                return nullptr;
            };
            auto registerCPU(cpu::abstractCPU *cpu) -> void override{};
        };

    } // namespace system
} // namespace archXplore
