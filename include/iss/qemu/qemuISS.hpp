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

                // auto receiveInstruction(cpu::instruction_t &insn) -> void override
                // {
                //     insn = sparta::allocate_sparta_shared_pointer<cpu::instruction_t>(m_insn_allocator, m_insn_queue->pop());
                // };

                auto generateFetchRequest(bool &enable_fetch) -> void override{
                    auto cpu = getCPUPtr();
                    // Update cpu status
                    auto ev = m_event_queue->front();
                    if(__glibc_unlikely(ev.tag == ev.ThreadApiTag || ev.tag == ev.SyscallApiTag)){
                        handleEvent(ev);
                    }
                    enable_fetch = true;
                    if(!cpu->isRunning() || cpu->isBlocked() || cpu->isCompleted()) {
                        enable_fetch = false;
                    }
                };

                auto receiveFetchResponse(instPtrBlock &insn_block) -> void override{
                    auto insn = sparta::allocate_sparta_shared_pointer<cpu::instruction_t>(m_insn_allocator, m_insn_queue->pop());
                };

            protected:
                auto handleEvent(const cpu::threadEvent_t &ev) -> void
                {
                    switch (ev.tag)
                    {
                    case ev.SyscallApiTag:
                        handleSyscallApi(ev.event_id, ev.syscall_api);
                        break;
                    case ev.ThreadApiTag:
                        handleThreadApi(ev.event_id, ev.thread_api);
                        break;
                    default:
                        return;
                        break;
                    }
                    m_event_queue->pop();
                };

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
