#pragma once

#include "iss/abstractISS.hpp"
#include "iss/qemu/qemuInterface.hpp"
#include "cpu/abstractCPU.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            class qemuISS : public abstractISS
            {
            public:
                qemuISS(){};
                ~qemuISS(){};

                auto _init() -> void override
                {
                    m_insn_queue = qemuInterface::getHartInsnQueuePtr(
                        m_cpu->getThreadID());
                };

                void receiveInstruction(isa::traceInsnPtr_t &insn) override
                {
                    insn = sparta::allocate_sparta_shared_pointer<isa::traceInsn_t>(m_insn_allocator, m_insn_queue->pop());
                };

            private:
                hartInsnQueue *m_insn_queue;
            };
        }
    } // namespace iss
} // namespace archXplore
