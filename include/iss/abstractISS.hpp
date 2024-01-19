#pragma once

#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaSharedPointerAllocator.hpp"

#include "iss/type.hpp"
#include "isa/traceInsn.hpp"

// Forward Declaration
namespace archXplore::cpu
{
    class abstractCPU;
}

namespace archXplore
{
    namespace iss
    {

        class abstractISS
        {
        public:
            using UniquePtr = std::unique_ptr<abstractISS>;
            // Delete copy-construct function
            abstractISS(const abstractISS &that) = delete;
            abstractISS &operator=(const abstractISS &that) = delete;

            abstractISS() : m_insn_allocator(10000, 10000){};

            ~abstractISS(){};

            virtual auto receiveInstruction(isa::traceInsnPtr_t &insn) -> void = 0;

            virtual auto _init() -> void = 0;

            auto init(cpu::abstractCPU *cpu) -> void;

        protected:
            cpu::abstractCPU *m_cpu;
            sparta::SpartaSharedPointerAllocator<isa::traceInsn_t> m_insn_allocator;
        };

    } // namespace iss
} // namespace archXplore
