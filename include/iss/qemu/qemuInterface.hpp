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

#include <iss/insnTunnel.hpp>
#include <iss/systemSyncEvent.hpp>
#include <isa/traceInsn.hpp>
#define MAX_HART 128

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            class hartInsnQueue;
            class qemuInterface;

            // QEMU Interface Instance variable
            extern std::mutex g_qemu_if_lock;
            extern std::shared_ptr<qemuInterface> g_qemu_if;

            // QEMU Thread
            extern std::thread *m_qemu_thread;
            // QEMU operation mutex
            extern std::mutex m_qemu_lock;
            extern std::shared_mutex m_qemu_sync_lock;
            extern std::condition_variable m_qemu_cond;
            // QEMU status
            extern std::atomic<eventId_t> m_qemu_sync_event_id;
            // Global Sync Event Register
            extern std::atomic<bool> m_qemu_sync_event_pending;
            extern systemSyncEvent_t m_qemu_sync_event;
            // Instruction Queue
            extern hartInsnQueue *m_qemu_insn_queue[MAX_HART];

            struct qemuArgs_t
            {
                int argc;
                char **argv;
                char **envp;
            };

            class hartInsnQueue
            {
            public:
                hartInsnQueue(const uint64_t &buffer_num = 1, const uint64_t &buffer_size = 16384)
                {
                    m_uid = 0;
                    m_insn_tunnel = std::make_unique<insnTunnel<archXplore::isa::traceInsn_t>>(buffer_num, buffer_size);
                };
                hartInsnQueue(hartInsnQueue &that)
                {
                    m_uid = 0;
                    m_insn_tunnel = std::move(that.m_insn_tunnel);
                };
                ~hartInsnQueue(){};
                auto inline pop() -> archXplore::isa::traceInsn_t
                {
                    archXplore::isa::traceInsn_t t;
                    m_insn_tunnel->pop(t);
                    return t;
                };
                auto inline isEmpty() -> bool
                {
                    return m_insn_tunnel->is_tunnel_empty();
                };

            protected:
                friend class qemuInterface;
                auto inline sendLastInsn() -> void
                {
                    if (m_last_exec_insn != nullptr)
                    {
                        m_insn_tunnel->push(*m_last_exec_insn.get());
                        m_last_exec_insn->mem.clear();
                    }
                };
                auto executeCallback(const addr_t &pc,
                                     const opcode_t &opcode, const uint8_t &len) -> void
                {
                    if (m_last_exec_insn == nullptr)
                    {
                        m_last_exec_insn = std::make_unique<archXplore::isa::traceInsn_t>();
                    }
                    else
                    {
                        m_last_exec_insn->target_pc = pc;
                        sendLastInsn();
                    }
                    m_last_exec_insn->uid = m_uid.fetch_add(1);
                    m_last_exec_insn->pc = pc;
                    m_last_exec_insn->opcode = opcode;
                    m_last_exec_insn->len = len;
                };
                auto memoryCallback(const addr_t &vaddr, const uint8_t &len) -> void
                {
                    m_last_exec_insn->mem.push_back({vaddr, len});
                };
                auto exitCallback() -> void
                {
                    sendLastInsn();
                    m_last_exec_insn.reset();
                    m_last_exec_insn = nullptr;
                    m_insn_tunnel->producer_exit();
                };

            private:
                std::atomic<eventId_t> m_uid;
                std::unique_ptr<archXplore::isa::traceInsn_t> m_last_exec_insn;
                std::unique_ptr<insnTunnel<archXplore::isa::traceInsn_t>> m_insn_tunnel;
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
                static auto inline getHartInsnQueuePtr(const hartId_t hart) -> hartInsnQueue *
                {
                    if (m_qemu_insn_queue[hart] == nullptr)
                    {
                        m_qemu_insn_queue[hart] = new hartInsnQueue();
                    }
                    return m_qemu_insn_queue[hart];
                };
                static auto fetchAddEventId() -> eventId_t
                {
                    return m_qemu_sync_event_id.fetch_add(1);
                };
                static auto pendingSyncEvent() -> bool
                {
                    return m_qemu_sync_event_pending.load(std::memory_order::memory_order_seq_cst);
                };
                static auto addSyncEvent(const systemSyncEvent_t &ev) -> void{
                    // Get Unique Lock to block qemu vCPU thread
                    {
                        std::unique_lock<std::shared_mutex> sync_lock(m_qemu_sync_lock);
                // Get Unique lock to write qemu-related structure
                {
                    std::unique_lock<std::mutex> lock(m_qemu_lock);
                    // Push sync event
                    m_qemu_sync_event = ev;
                    m_qemu_sync_event_pending.store(true, std::memory_order::memory_order_seq_cst);
                    // Wait for processing and wakeup
                    while (pendingSyncEvent())
                    {
                        m_qemu_cond.wait(lock);
                    };
                }
            }
        };
        static auto getPendingSyncEvent() -> systemSyncEvent_t
        {
            return m_qemu_sync_event;
        };
        static auto removeSyncEvent() -> void{
            // Get Unique lock to write qemu-related structure
            {
                std::unique_lock<std::mutex> lock(m_qemu_lock);
        m_qemu_sync_event_pending.store(false, std::memory_order::memory_order_seq_cst);
        // Wait up sync event owner
        m_qemu_cond.notify_one();
    }
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
    auto vcpu_queue = getHartInsnQueuePtr(vcpu_index);
    // // Add CPU Init Event
    systemSyncEvent_t ev;
    ev.event_id = fetchAddEventId();
    ev.event_type = systemSyncEventTypeEnum_t::hartInit;
    ev.hart_id = vcpu_index;
    addSyncEvent(ev);
};
static auto qemu_vcpu_exit(qemu_plugin_id_t id, unsigned int vcpu_index) -> void{
    // std::cout << "VCPU exitted " << vcpu_index << std::endl;
};
static auto qemu_shutdown(int exit_code = 0) -> void
{
    removeSyncEvent();
    qemu_plugin_shutdown(exit_code);
    qemuThreadJoin();
};
static auto qemu_exit(qemu_plugin_id_t id, void *userdata) -> void
{
    for (size_t i = 0; i < MAX_HART; i++)
    {
        if (m_qemu_insn_queue[i] != nullptr)
        {
            m_qemu_insn_queue[i]->exitCallback();
        }
        else
        {
            break;
        }
    }
    // // Add QEMU Exit Event
    systemSyncEvent_t ev;
    ev.event_id = fetchAddEventId();
    ev.event_type = systemSyncEventTypeEnum_t::systemExit;
    addSyncEvent(ev);
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
    hartInsnQueue *insn_queue = getHartInsnQueuePtr(vcpu_index);
    qemu_plugin_insn *insn = (qemu_plugin_insn *)userdata;
    // Acquire sync lock
    {
        std::shared_lock<std::shared_mutex> lock(m_qemu_sync_lock);
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
        insn_queue->executeCallback(
            pc,
            opcode,
            len);
    }
};
static auto qemu_vcpu_mem(unsigned int vcpu_index,
                          qemu_plugin_meminfo_t info,
                          uint64_t vaddr,
                          void *userdata) -> void
{
    hartInsnQueue *insn_queue = getHartInsnQueuePtr(vcpu_index);
    const qemu_plugin_hwaddr *haddr = qemu_plugin_get_hwaddr(info, vaddr);
    // const addr_t paddr = qemu_plugin_hwaddr_phys_addr(haddr);
    const uint8_t len = qemu_plugin_mem_size_shift(info);
    insn_queue->memoryCallback(
        vaddr, len);
};
}
;
}
}
}