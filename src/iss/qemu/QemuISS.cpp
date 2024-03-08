#include "iss/qemu/QemuISS.hpp"
#include "cpu/AbstractCPU.hpp"
#include "system/AbstractSystem.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            auto QemuISS::initCPUState() -> void
            {
                // 0. Aquire the first event from the event queue
                auto& first_event = m_event_queue->front();
                sparta_assert(first_event.tag == first_event.InsnTag, "First event is not an instruction");
                // 1. Initialize boot PC
                m_cpu->m_boot_pc = first_event.instruction.pc;
                // 2. Set CPU status to active
                m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
            };

            auto QemuISS::generateFetchRequest(const Addr_t &addr, const size_t &fetch_size) -> void
            {
                // Exit loop flag
                bool exit_loop = false;
                // Prefetch the next instruction package here
                Addr_t cur_fetch_pc = addr;
                std::vector<cpu::StaticInst_t> fetch_package;
                while (cur_fetch_pc < addr + fetch_size)
                {
                    bool do_pop = true;
                    cpu::ThreadEvent_t& ev = m_event_queue->front();
                    if (SPARTA_EXPECT_FALSE(ev.tag == cpu::ThreadEvent_t::ThreadApiTag))
                    {
                        handleThreadApi(ev);
                        exit_loop = true;
                    }
                    else if (SPARTA_EXPECT_FALSE(ev.tag == cpu::ThreadEvent_t::SyscallApiTag))
                    {
                        handleSyscallApi(ev);
                        exit_loop = true;
                    }
                    else
                    {
                        auto& inst = ev.instruction;
                        if(ev.is_last == true)
                        {
                            
                        }
                        if ((cur_fetch_pc == inst.pc) && (cur_fetch_pc + inst.len <= addr + fetch_size)) 
                        {
                            fetch_package.emplace_back(inst);
                            cur_fetch_pc += inst.len;
                        } else {
                            exit_loop = true;
                            do_pop = false;
                        }
                    }
                    if(do_pop)
                    {
                        if(SPARTA_EXPECT_FALSE(ev.is_last))
                        {
                            m_cpu->m_status = cpu::cpuStatus_t::COMPLETED;
                            m_cpu->cancelNextTickEvent();
                        }
                        m_event_queue->popFront();
                    }
                    if(SPARTA_EXPECT_FALSE(exit_loop))
                    {
                        break;
                    }
                }
                m_fetch_buffer.emplace_back(fetch_package);
            };

            auto QemuISS::processFetchResponse(const uint8_t *data) -> std::vector<cpu::StaticInst_t>
            {
                sparta_assert(!m_fetch_buffer.empty(), "Fetch buffer is empty");
                auto inst_group = m_fetch_buffer.front();
                m_fetch_buffer.pop_front();
                return inst_group;
            };

            auto QemuISS::handleThreadApi(const cpu::ThreadEvent_t& ev) -> void{

            };

            auto QemuISS::handleSyscallApi(const cpu::ThreadEvent_t& ev) -> void
            {
                if (m_cpu->m_status == cpu::cpuStatus_t::ACTIVE)
                {
                    m_cpu->m_status = cpu::cpuStatus_t::BLOCKED_SYSCALL;
                    m_cpu->cancelNextTickEvent();
                    m_cpu->scheduleWakeUpMonitorEvent();
                }
            };

            auto QemuISS::wakeUpMonitor() -> void
            {
                switch (m_cpu->m_status)
                {
                case cpu::cpuStatus_t::INACTIVE:
                    // TODO : temporary solution, need to implement pthread_create API
                    if (m_event_queue->tryTake())
                    {
                        m_cpu->startUp();
                        m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        m_cpu->cancelWakeUpMonitorEvent();
                        m_cpu->scheduleNextTickEvent();
                    }
                    // else if (m_event_queue->isCompleted())
                    // {
                    //     m_cpu->m_status = cpu::cpuStatus_t::COMPLETED;
                    //     m_cpu->cancelWakeUpMonitorEvent();
                    // }
                    break;
                case cpu::cpuStatus_t::BLOCKED_SYSCALL:
                    if (m_event_queue->tryTake())
                    {
                        m_cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        m_cpu->cancelWakeUpMonitorEvent();
                        m_cpu->scheduleNextTickEvent();
                    }
                default:
                    break;
                }
            };

            auto QemuISS::initialize() -> void
            {
                const auto &hart_id = m_cpu->getHartID();

                const auto app_name = m_cpu->getSystemPtr()->getAppName();

                m_event_queue = std::make_unique<EventSubscriber>(app_name, hart_id);
            };

            QemuISS::QemuISS() = default;

            QemuISS::~QemuISS() = default;
        }
    }
}