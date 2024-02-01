#pragma once

extern "C"
{
#include <iss/qemu/qemuEmulator.h>
#include <iss/qemu/qemu-plugin.h>
}

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

#include "types.hpp"
#include "cpu/threadEvent.hpp"
#include "utils/threadSafeQueue.hpp"
#include "system/abstractSystem.hpp"

#define MAX_HART 128

using namespace archXplore::utils;

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            class hartEventQueue;
            class qemuInterface;

            // QEMU Interface Instance variable
            extern std::mutex g_qemu_if_lock;
            extern std::shared_ptr<qemuInterface> g_qemu_if;

            // QEMU Thread
            extern std::thread *m_qemu_thread;
            // QEMU operation mutex
            extern std::mutex m_qemu_lock;
            extern std::condition_variable m_qemu_cond;
            // Instruction Queue
            extern hartEventQueue *m_qemu_event_queue[MAX_HART];

            struct qemuArgs_t
            {
                int argc;
                char **argv;
                char **envp;
            };

            class hartEventQueue
            {
            public:
                // Deleted copy constructor
                hartEventQueue(const hartEventQueue &that) = delete;
                hartEventQueue &operator=(const hartEventQueue &that) = delete;

                // Constructor
                hartEventQueue(const hartId_t hart_id, const std::size_t &queue_size = 16384)
                    : m_event_id(0), m_insn_uid(0), m_buffer_size(queue_size), m_hart_id(hart_id){};

                // Destructor
                ~hartEventQueue() = default;

            public:
                auto front() -> cpu::threadEvent_t &
                {
                    if (__glibc_unlikely(m_local_pop_event_buffer.size() == 0))
                    {
                        syncPopBuffer();
                    }
                    return m_local_pop_event_buffer.front();
                };
                auto pop() -> void
                {
                    m_local_pop_event_buffer.pop_front();
                };

            protected:
                friend class qemuInterface;
                auto inline sendLastInsn() -> void
                {
                    m_local_push_event_buffer.emplace_back(cpu::threadEvent_t::InsnTag, m_event_id,
                                                           *m_last_exec_insn);
                    m_last_exec_insn->clear();
                };
                auto executeCallback(const addr_t &pc,
                                     const opcode_t &opcode, const uint8_t &len) -> void
                {
                    if (__glibc_unlikely(m_last_exec_insn == nullptr))
                    {
                        m_last_exec_insn = std::make_unique<cpu::instruction_t>();
                    }
                    else
                    {
                        m_last_exec_insn->br_info.target_pc = pc;
                        sendLastInsn();
                    }
                    m_last_exec_insn->uid = fetchAddEventId(m_insn_uid);
                    m_last_exec_insn->pc = pc;
                    m_last_exec_insn->opcode = opcode;
                    m_last_exec_insn->len = len;
                };
                auto memoryCallback(const addr_t &vaddr, const uint8_t &len) -> void
                {
                    m_last_exec_insn->mem.push_back({vaddr, len});
                };
                auto initCallback() -> void
                {
                    auto system = system::abstractSystem::getSystemPtr();
                    auto cpu = system->getCPUPtr(m_hart_id);
                    std::lock_guard<std::mutex> lock(system->m_system_lock);
                    {
                        cpu->m_status = cpu::cpuStatus_t::ACTIVE;
                        cpu->m_cycle = 0;
                        cpu->m_instret = 0;
                    }
                };
                auto exitCallback() -> void
                {
                    // Set last instruciton flag
                    m_last_exec_insn->is_last = true;
                    // Send last instruction in buffer
                    sendLastInsn();
                    // Sync push buffer
                    syncPushBuffer(true);
                };
                inline auto fetchAddEventId(eventId_t &id) -> eventId_t
                {
                    auto cur_id = id;
                    id = id + 1;
                    return cur_id;
                };
                inline auto syncPushBuffer(const bool &force = false) -> void
                {
                    // When buffer full, clear all elements to
                    if ((m_local_push_event_buffer.size() == m_buffer_size) || force)
                    {
                        m_thread_event_queue.pushBatch(m_local_push_event_buffer);
                    };
                };
                inline auto syncPopBuffer(const bool &force = false) -> void
                {
                    if (m_local_pop_event_buffer.size() == 0 || force)
                    {
                        m_thread_event_queue.popBatch(m_local_pop_event_buffer, m_buffer_size);
                    };
                };

            private:
                // Buffer size
                const std::size_t m_buffer_size;
                // Hart Id
                const hartId_t m_hart_id;
                // Event id
                eventId_t m_event_id;
                eventId_t m_insn_uid;
                std::unique_ptr<cpu::instruction_t> m_last_exec_insn;
                std::deque<cpu::threadEvent_t> m_local_push_event_buffer;
                std::deque<cpu::threadEvent_t> m_local_pop_event_buffer;
                utils::threadSafeQueue<cpu::threadEvent_t> m_thread_event_queue;
            };

            class qemuInterface
            {
            public:
                using ptrType = std::shared_ptr<qemuInterface>;
                // Deleted function
                qemuInterface(qemuInterface &) = delete;
                qemuInterface(const qemuInterface &) = delete;
                qemuInterface &operator=(const qemuInterface &) = delete;
                // Default constructor
                qemuInterface(){};
                // Deconstructor
                ~qemuInterface(){};
                // Entry function to start qemu thread
                auto bootQemuThread(const qemuArgs_t &args) -> void
                {
                    std::lock_guard<std::mutex> lock(g_qemu_if_lock);
                    if (m_qemu_thread == nullptr)
                    {
#ifndef CONFIG_USER_ONLY
                        m_qemu_thread = new std::thread(qemuSystemEmulator, args.argc, args.argv);
#else
                        m_qemu_thread = new std::thread(qemuUserEmulator, args.argc, args.argv, args.envp);
#endif
                    };
                };
                // Block QEMU thread
                auto blockQemuThread() -> void
                {
                    // Lock QEMU IO thread
                    qemu_plugin_mutex_lock_iothread();
                };
                // Unblock QEMU thread
                auto unblockQemuThread() -> void
                {
                    // Unlock QEMU IO thread
                    qemu_plugin_mutex_unlock_iothread();
                };
                // Qemu thread guard
                static auto qemuThreadJoin() -> void
                {
                    if (m_qemu_thread->joinable())
                    {
                        m_qemu_thread->join();
                    }
                };
                static auto inline getHartEventQueuePtr(const hartId_t hart) -> hartEventQueue *
                {
                    if (m_qemu_event_queue[hart] == nullptr)
                    {
                        m_qemu_event_queue[hart] = new hartEventQueue(hart);
                    }
                    return m_qemu_event_queue[hart];
                };
                // The only entry to get/construct qemuInterface
                static auto getInstance() -> std::shared_ptr<qemuInterface>
                {
                    std::lock_guard<std::mutex> lock(g_qemu_if_lock);
                    if (g_qemu_if == nullptr)
                    {
                        g_qemu_if = std::make_shared<qemuInterface>();
                    }
                    return g_qemu_if;
                };
                /* Instrument functions for QEMU */
                static auto qemu_vcpu_init(qemu_plugin_id_t id, unsigned int vcpu_index) -> void
                {
                    // Resize hart instruction queue
                    auto vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    vcpu_queue->initCallback();
                };
                static auto qemu_vcpu_exit(qemu_plugin_id_t id, unsigned int vcpu_index) -> void{
                    // std::cout << "VCPU exitted " << vcpu_index << std::endl;
                };
                static auto qemu_shutdown(int exit_code = 0) -> void
                {
                    qemu_plugin_shutdown(exit_code);
                    qemuThreadJoin();
                };
                static auto qemu_exit(qemu_plugin_id_t id, void *userdata) -> void
                {
                    for (auto vcpu_queue : m_qemu_event_queue)
                    {
                        if (vcpu_queue)
                        {
                            vcpu_queue->exitCallback();
                        }
                        else
                        {
                            break;
                        }
                    }
                };
                static auto qemu_vcpu_tb_trans(qemu_plugin_id_t id, qemu_plugin_tb *tb) -> void
                {
                    qemu_plugin_insn *insn;
                    for (size_t i = 0; i < qemu_plugin_tb_n_insns(tb); ++i)
                    {
                        insn = qemu_plugin_tb_get_insn(tb, i);
                        // Register Instruction Execution Callback
                        qemu_plugin_register_vcpu_insn_exec_cb(insn, qemu_vcpu_insn_exec, QEMU_PLUGIN_CB_NO_REGS, (void *)insn);
                        // Register Instruction Memory Callback
                        qemu_plugin_register_vcpu_mem_cb(insn, qemu_vcpu_mem, QEMU_PLUGIN_CB_NO_REGS, QEMU_PLUGIN_MEM_RW, NULL);
                    }
                };
                static auto qemu_vcpu_insn_exec(unsigned int vcpu_index, void *userdata) -> void
                {
                    hartEventQueue *vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    qemu_plugin_insn *insn = (qemu_plugin_insn *)userdata;
                    // Acquire sync lock
                    const qemu_plugin_hwaddr *insn_haddr = (const qemu_plugin_hwaddr *)qemu_plugin_insn_haddr(insn);
                    const size_t len = qemu_plugin_insn_size(insn);
                    const addr_t pc = qemu_plugin_insn_vaddr(insn);
                    // const addr_t pc_paddr = qemu_plugin_hwaddr_phys_addr(insn_haddr);
                    opcode_t opcode;
                    switch (len)
                    {
                    case 1:
                        opcode = *((uint8_t *)qemu_plugin_insn_data(insn));
                        break;
                    case 2:
                        opcode = *((uint16_t *)qemu_plugin_insn_data(insn));
                        break;
                    case 4:
                        opcode = *((uint32_t *)qemu_plugin_insn_data(insn));
                        break;
                    case 8:
                        opcode = *((uint64_t *)qemu_plugin_insn_data(insn));
                        break;
                    default:
                        throw "Unknown instruction size!\n";
                        break;
                    }
                    vcpu_queue->executeCallback(
                        pc,
                        opcode,
                        len);
                };
                static auto qemu_vcpu_mem(unsigned int vcpu_index,
                                          qemu_plugin_meminfo_t info,
                                          uint64_t vaddr,
                                          void *userdata) -> void
                {
                    hartEventQueue *vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    const qemu_plugin_hwaddr *haddr = qemu_plugin_get_hwaddr(info, vaddr);
                    // const addr_t paddr = qemu_plugin_hwaddr_phys_addr(haddr);
                    const uint8_t len = qemu_plugin_mem_size_shift(info);
                    vcpu_queue->memoryCallback(
                        vaddr, len);
                };
            };
        };
    };
};
