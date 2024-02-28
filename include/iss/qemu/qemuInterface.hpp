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
#include <unordered_map>
#include <cassert>

#include "types.hpp"
#include "cpu/threadEvent.hpp"
#include "utils/threadSafeQueue.hpp"

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
            extern bool m_qemu_exited;
            extern hartId_t m_simulated_cpu_number;
            // Instruction Queue
            extern hartEventQueue *m_qemu_event_queue[MAX_HART];

            struct qemu_plugin_insn_t
            {
                addr_t pc;
                opcode_t opcode;
                uint8_t len;
            };

            // QEMU plugin instruction map
            static std::unordered_map<addr_t, qemu_plugin_insn_t> m_plugin_insn_map;
            static std::shared_mutex m_plugin_insn_map_lock;

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
                    : m_event_id(0), m_insn_uid(0), m_buffer_size(queue_size), m_hart_id(hart_id),
                      m_initted(false), m_completed(false)
                {
                    m_local_push_event_buffer.reserve(queue_size * 5);

                    m_local_pop_event_buffer.reserve(queue_size * 5);

                    m_thread_event_queue.setCapacity(queue_size * 5);
                };

                // Destructor
                ~hartEventQueue() = default;

            public:
                // Non-blocking one compared to front()
                auto isInitted() -> bool
                {
                    return m_initted;
                };
                auto isCompleted() -> bool
                {
                    return m_completed || m_qemu_exited;
                };
                auto front() -> cpu::threadEvent_t &
                {
                    if (!m_local_pop_event_buffer.size())
                    {
                        m_thread_event_queue.popBatch(m_local_pop_event_buffer);
                    }
                    return m_local_pop_event_buffer.back();
                };
                auto pop() -> void
                {
                    m_local_pop_event_buffer.pop_back();
                };

            protected:
                friend class qemuInterface;
                inline auto sendLastInsn(bool is_last = false) -> void
                {
                    // Set last instruction flag
                    m_last_exec_insn->is_last = is_last;
                    // push instruction
                    m_local_push_event_buffer.emplace_back(cpu::threadEvent_t::InsnTag, fetchAddEventId(m_event_id),
                                                           *m_last_exec_insn);
                    m_last_exec_insn->clear();
                    // Push to thread queue
                    if(__glibc_unlikely(is_last))
                    {
                        m_thread_event_queue.pushBatch(m_local_push_event_buffer);
                    } else if(m_local_push_event_buffer.size() >= m_buffer_size)
                    {
                        m_thread_event_queue.tryPushBatch(m_local_push_event_buffer);
                    }
                };
                inline auto executeCallback(const addr_t &pc,
                                            const opcode_t &opcode, const uint8_t &len) -> void
                {
                    if (__glibc_unlikely(m_last_exec_insn == nullptr))
                    {
                        m_last_exec_insn = std::make_unique<cpu::instruction_t>();
                    }
                    else
                    {
                        m_last_exec_insn->br_info.target_pc = pc;
                        sendLastInsn(false);
                    }
                    m_last_exec_insn->uid = fetchAddEventId(m_insn_uid);
                    m_last_exec_insn->pc = pc;
                    m_last_exec_insn->opcode = opcode;
                    m_last_exec_insn->len = len;
                };
                inline auto memoryCallback(const addr_t &vaddr, const uint8_t &len, const bool &is_store) -> void
                {
                    m_last_exec_insn->mem.push_back({vaddr, len, is_store});
                    // Construct memory dependency
                    for (auto addr = vaddr; addr < vaddr + len * 8; addr += 8)
                    {
                        auto it = m_mem_dep_table.find(addr);
                        if (it != m_mem_dep_table.end())
                        { // Dependency found for this address
                            if (is_store)
                            {
                                // Construct WAW dependency
                                m_last_exec_insn->mem_dep.push_back({it->second, cpu::instruction_t::WAW});
                            }
                            else
                            {
                                // Construct RAW dependency
                                m_last_exec_insn->mem_dep.push_back({it->second, cpu::instruction_t::RAW});
                            }
                        }
                        // Update memory dependency table
                        if (is_store)
                        {
                            m_mem_dep_table[addr] = m_event_id;
                        }
                    }
                };
                inline auto initCallback() -> void
                {
                    m_initted = true;
                };
                inline auto exitCallback() -> void
                {
                    if (m_initted)
                    {
                        // Send last instruction in buffer
                        sendLastInsn(true);
                    }
                    m_completed = true;
                };
                inline auto fetchAddEventId(eventId_t &id) -> eventId_t
                {
                    auto cur_id = id;
                    id = id + 1;
                    return cur_id;
                };

            private:
                // Status
                bool m_initted;
                bool m_completed;
                // Buffer size
                const std::size_t m_buffer_size;
                // Hart Id
                const hartId_t m_hart_id;
                // Event id
                eventId_t m_event_id;
                eventId_t m_insn_uid;
                std::unique_ptr<cpu::instruction_t> m_last_exec_insn;
                std::vector<cpu::threadEvent_t> m_local_push_event_buffer;
                std::vector<cpu::threadEvent_t> m_local_pop_event_buffer;
                utils::threadSafeQueue<cpu::threadEvent_t> m_thread_event_queue;
                // Memory dependency table(Per Byte)
                std::unordered_map<addr_t, eventId_t> m_mem_dep_table;
                // Integer Register dependency table(Per Register)
                std::unordered_map<uint8_t, eventId_t> m_int_reg_dep_table;
                // Float Register dependency table(Per Register)
                std::unordered_map<uint8_t, eventId_t> m_float_reg_dep_table;
                // Vector Register dependency table(Per Register)
                std::unordered_map<uint8_t, eventId_t> m_vec_reg_dep_table;
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
                    if (vcpu_index >= m_simulated_cpu_number)
                    {
                        throw std::out_of_range("Simulated CPUs are less than vcpu needed by QEMU\n");
                    }
                    // Resize hart instruction queue
                    auto vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    vcpu_queue->initCallback();
                };
                static auto qemu_vcpu_exit(qemu_plugin_id_t id, unsigned int vcpu_index) -> void
                {
                    auto vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    vcpu_queue->exitCallback();
                };
                static auto qemu_exit(qemu_plugin_id_t id, void *userdata) -> void
                {
                    m_qemu_exited = true;
                    pthread_exit(NULL);
                };
                static auto qemu_vcpu_tb_trans(qemu_plugin_id_t id, qemu_plugin_tb *tb) -> void
                {
                    qemu_plugin_insn *insn;
                    for (size_t i = 0; i < qemu_plugin_tb_n_insns(tb); ++i)
                    {
                        insn = qemu_plugin_tb_get_insn(tb, i);
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
                        {
                            std::unique_lock<std::shared_mutex> lock(m_plugin_insn_map_lock);
                            m_plugin_insn_map[pc] = {pc, opcode, (uint8_t)len};
                            // Register Instruction Execution Callback
                            qemu_plugin_register_vcpu_insn_exec_cb(insn, qemu_vcpu_insn_exec, QEMU_PLUGIN_CB_NO_REGS, (void *)(&m_plugin_insn_map[pc]));
                            // Register Instruction Memory Callback
                            qemu_plugin_register_vcpu_mem_cb(insn, qemu_vcpu_mem, QEMU_PLUGIN_CB_NO_REGS, QEMU_PLUGIN_MEM_RW, NULL);
                        }
                    }
                };

                static auto qemu_vcpu_insn_exec(unsigned int vcpu_index, void *userdata) -> void
                {
                    hartEventQueue *vcpu_queue = getHartEventQueuePtr(vcpu_index);
                    qemu_plugin_insn_t insn;
                    {
                        std::shared_lock<std::shared_mutex> lock(m_plugin_insn_map_lock);
                        insn = *(qemu_plugin_insn_t *)userdata;
                    }
                    vcpu_queue->executeCallback(
                        insn.pc,
                        insn.opcode,
                        insn.len);
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
                    const bool is_store = qemu_plugin_mem_is_store(info);
                    vcpu_queue->memoryCallback(
                        vaddr, len, is_store);
                };
            };
        };
    };
};
