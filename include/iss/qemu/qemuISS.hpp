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
                qemuISS() = default;
                ~qemuISS() = default;

                auto readyToPowerOn() -> bool override 
                {
                    return m_event_queue->peek();
                };

                auto generateFetchRequest() -> sparta::SpartaSharedPointer<cpu::instruction_t> override
                {
                    auto cpu = getCPUPtr();
                    // Update cpu status
                    auto ev = m_event_queue->front();
                    // Instruction Ptr
                    sparta::SpartaSharedPointer<cpu::instruction_t> insn;
                    // Continue flag
                    bool continue_flag = true;
                    while (continue_flag)
                    {
                        if (ev.tag == ev.SyscallApiTag)
                        {
                            handleSyscallApi(ev.event_id, ev.syscall_api);
                        }
                        else if (ev.tag == ev.ThreadApiTag)
                        {
                            handleThreadApi(ev.event_id, ev.thread_api);
                        }
                        else
                        {
                            insn = sparta::allocate_sparta_shared_pointer<cpu::instruction_t>(m_insn_allocator, ev.instruction);
                            continue_flag = false;
                        }
                        m_event_queue->pop();
                    }
                    return insn;
                };

            protected:
                auto handleThreadApi(const eventId_t &id, const cpu::threadApi_t &api) -> void
                {
                    auto cpu = getCPUPtr();
                    switch (api.eventType)
                    {
                    case cpu::threadApi_t::eventType_t::THREAD_CREATE:
                        cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        cpu->m_cycle = 0;
                        cpu->m_instret = 0;
                        break;
                    default:
                        break;
                    }
                };

                auto handleSyscallApi(const eventId_t &id, const cpu::syscallApi_t &api) -> void
                {
                    return;
                };

                auto init() -> void override
                {
                    m_event_queue = qemuInterface::getHartEventQueuePtr(
                        m_cpu->getThreadID());
                };

            private:
                hartEventQueue *m_event_queue;
            };
        }
    } // namespace iss
} // namespace archXplore
