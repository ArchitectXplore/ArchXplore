#include "iss/qemu/qemuISS.hpp"
#include "cpu/abstractCPU.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            auto qemuISS::initCPUState() -> void
            {
                // 0. Aquire the first event from the event queue
                auto& first_event = m_event_queue->front();
                sparta_assert(first_event.tag == first_event.InsnTag, "First event is not an instruction");
                // 1. Initialize boot PC
                m_cpu->m_boot_pc = first_event.instruction.pc;
                // 2. Set CPU status to active
                m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
            };

            auto qemuISS::generateFetchRequest(const addr_t &addr, const size_t &fetch_size) -> void
            {
                // Prefetch the next instruction package here
                addr_t cur_fetch_pc = addr;
                std::vector<cpu::instruction_t> fetch_package;
                while (cur_fetch_pc < addr + fetch_size)
                {
                    cpu::threadEvent_t& ev = m_event_queue->front();
                    if (SPARTA_EXPECT_FALSE(ev.tag == cpu::threadEvent_t::ThreadApiTag))
                    {
                        handleThreadApi(ev);
                        m_event_queue->popFront();
                        break;
                    }
                    else if (SPARTA_EXPECT_FALSE(ev.tag == cpu::threadEvent_t::SyscallApiTag))
                    {
                        handleSyscallApi(ev);
                        m_event_queue->popFront();
                        break;
                    }
                    else
                    {
                        auto& inst = ev.instruction;
                        if ((cur_fetch_pc == inst.pc) && (cur_fetch_pc + inst.len <= addr + fetch_size)) 
                        {
                            fetch_package.emplace_back(inst);
                            cur_fetch_pc += inst.len;
                            if (SPARTA_EXPECT_FALSE(ev.instruction.is_last))
                            {
                                m_cpu->m_status = cpu::cpuStatus_t::COMPLETED;
                                m_cpu->cancelNextTickEvent();
                            }
                            m_event_queue->popFront();
                        } else {
                            break;
                        }
                    }
                }
                m_fetch_buffer.emplace_back(fetch_package);
            };

            auto qemuISS::processFetchResponse(const uint8_t *data) -> std::vector<cpu::instruction_t>
            {
                sparta_assert(!m_fetch_buffer.empty(), "Fetch buffer is empty");
                auto inst_group = m_fetch_buffer.front();
                m_fetch_buffer.pop_front();
                return inst_group;
            };

            auto qemuISS::handleThreadApi(const cpu::threadEvent_t& ev) -> void{

            };

            auto qemuISS::handleSyscallApi(const cpu::threadEvent_t& ev) -> void
            {
                if (m_cpu->m_status == cpu::cpuStatus_t::ACTIVE)
                {
                    m_cpu->m_status = cpu::cpuStatus_t::BLOCKED_SYSCALL;
                    m_cpu->cancelNextTickEvent();
                    m_cpu->scheduleWakeUpMonitorEvent();
                }
            };

            auto qemuISS::wakeUpMonitor() -> void
            {
                switch (m_cpu->m_status)
                {
                case cpu::cpuStatus_t::INACTIVE:
                    // TODO : temporary solution, need to implement pthread_create API
                    if (m_event_queue->isInitted())
                    {
                        m_cpu->startUp();
                        m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        m_cpu->cancelWakeUpMonitorEvent();
                        m_cpu->scheduleNextTickEvent();
                    }
                    else if (m_event_queue->isCompleted())
                    {
                        m_cpu->m_status = cpu::cpuStatus_t::COMPLETED;
                        m_cpu->cancelWakeUpMonitorEvent();
                    }
                    break;
                case cpu::cpuStatus_t::BLOCKED_SYSCALL:
                    if (!m_event_queue->isEmpty())
                    {
                        m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        m_cpu->cancelWakeUpMonitorEvent();
                        m_cpu->scheduleNextTickEvent();
                    }
                default:
                    break;
                }
            };

            auto qemuISS::initialize() -> void
            {
                const auto &tid = m_cpu->getThreadID();
                m_event_queue = qemuInterface::getHartEventQueuePtr(tid);
            };

            qemuISS::qemuISS() = default;

            qemuISS::~qemuISS() = default;
        }
    }
}