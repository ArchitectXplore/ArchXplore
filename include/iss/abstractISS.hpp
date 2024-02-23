#pragma once

#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaSharedPointerAllocator.hpp"

#include "types.hpp"
#include "cpu/threadEvent.hpp"

// Forward Declaration
namespace archXplore::cpu
{
    class abstractCPU;
}

namespace archXplore
{
    namespace iss
    {
        using instPtr = sparta::SpartaSharedPointer<cpu::instruction_t>;
        using instPtrBlock = std::vector<instPtr>;

        class abstractISS
        {
        public:
            using UniquePtr = std::unique_ptr<abstractISS>;
            // Delete copy-construct function
            abstractISS(const abstractISS &that) = delete;
            abstractISS &operator=(const abstractISS &that) = delete;

            abstractISS() : m_insn_allocator(10000, 10000){};

            ~abstractISS(){};

            virtual auto generateFetchRequest() -> sparta::SpartaSharedPointer<cpu::instruction_t> = 0;

            // virtual auto receiveFetchResponse(instPtrBlock &insn_block) -> void = 0;

            virtual auto readyToPowerOn() -> bool {return true;};

            virtual auto readyToPowerOff() -> bool {return true;};

            virtual auto init() -> void = 0;

            auto getCPUPtr() -> cpu::abstractCPU *
            {
                return m_cpu;
            };

            auto setCPU(cpu::abstractCPU *cpu) -> void;

        protected:
            cpu::abstractCPU *m_cpu;
            sparta::SpartaSharedPointerAllocator<cpu::instruction_t> m_insn_allocator;
        };

    } // namespace iss
} // namespace archXplore
