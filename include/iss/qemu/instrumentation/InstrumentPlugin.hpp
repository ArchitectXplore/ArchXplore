#pragma once

extern "C"
{
#include "iss/qemu/include/qemu-plugin.h"
}

#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <iostream>

#include "cpu/StaticInst.hpp"
#include "iss/EventPublisher.hpp"

namespace archXplore
{

    namespace iss
    {

        namespace qemu
        {

            class InstrumentPlugin
            {
            public:
                /*
                 * @brief Instruction cache entry
                 */
                struct InstCacheEntry_t
                {
                    Addr_t pc;
                    uint32_t opcode;
                    uint8_t len;
                };

                typedef std::unordered_map<Addr_t, InstCacheEntry_t> InstCache_t;

                /**
                 * @brief Start publish service
                 *
                 * @return void
                 */
                static auto startPublishService() -> void
                {
                    // Initialize RouDi App
                    auto runtime_name = iox::RuntimeName_t(iox::TruncateToCapacity, m_runtime.c_str());
                    iox::runtime::PoshRuntime::initRuntime(runtime_name);
                    // Preallocate memory for possible harts
                    for(HartID_t i = 0; i < m_max_harts; ++i)
                    {
                        m_event_publishers[i] = nullptr;
                        m_last_insts[i] = cpu::StaticInst_t();
                        m_inst_counters[i] = 0;
                        m_event_counters[i] = 0;
                    }
                };

                /**
                 * @brief Calculate hart ID
                 * @param vcpu_index The index of the VCPU
                 *
                 * @return HartID_t
                 */
                static auto calculateHartID(unsigned int vcpu_index) -> const HartID_t
                {
                    return vcpu_index + m_boot_hart;
                };

                /**
                 * @brief Thread initialization
                 * @param id The QEMU plugin ID
                 * @param vcpu_index The index of the VCPU
                 *
                 * @return void
                 */
                static auto threadInitialize(qemu_plugin_id_t id, unsigned int vcpu_index) -> void
                {
                    assert(vcpu_index < m_max_harts);
                    // Calculate hart ID
                    const HartID_t hart_id = calculateHartID(vcpu_index);
                    // Create event publisher
                    m_event_publishers.at(vcpu_index) = std::make_unique<EventPublisher>(m_app_name, hart_id);
                };

                /**
                 * @brief Thread exit
                 * @param id The QEMU plugin ID
                 * @param vcpu_index The index of the VCPU
                 *
                 * @return void
                 */
                static auto threadExit(qemu_plugin_id_t id, unsigned int vcpu_index) -> void
                {
                    auto last_event = cpu::ThreadEvent_t(cpu::ThreadEvent_t::InsnTag, m_event_counters.at(vcpu_index)++,
                                                         m_last_insts.at(vcpu_index));
                    last_event.is_last = true;
                    // Send instruction
                    m_event_publishers.at(vcpu_index)->publish(true, last_event);
                };

                /**
                 * @brief QEMU at exit
                 * @param id The QEMU plugin ID
                 * @param userdata The userdata pointer
                 *
                 * @return void
                 */
                static auto qemuAtExit(qemu_plugin_id_t id, void *userdata) -> void
                {
                    userExitCallback(0);
                };

                /**
                 * @brief Publisher clean up
                 *
                 * @return void
                 */
                static auto userExitCallback(int signum) noexcept -> void
                {
                    // Release publish before exit(Roudi receives runtime close before publisher destruction)
                    for(auto &publisher : m_event_publishers)
                    {
                        publisher.second.reset();
                    }
                    std::exit(signum);
                };

                /**
                 * @brief Syscall
                 *
                 * @param id The QEMU plugin ID
                 * @param vcpu_index The index of the VCPU
                 * @param num The syscall number
                 * @param a1 The first argument
                 * @param a2 The second argument
                 * @param a3 The third argument
                 * @param a4 The fourth argument
                 * @param a5 The fifth argument
                 * @param a6 The sixth argument
                 * @param a7 The seventh argument
                 * @param a8 The eighth argument
                 *
                 * @return void
                 */
                static auto syscall(qemu_plugin_id_t id, unsigned int vcpu_index,
                                    int64_t num, uint64_t a1, uint64_t a2,
                                    uint64_t a3, uint64_t a4, uint64_t a5,
                                    uint64_t a6, uint64_t a7, uint64_t a8) -> void
                {
                    // Send syscall event
                    m_event_publishers.at(vcpu_index)->publish(
                        true, cpu::ThreadEvent_t::SyscallApiTag, m_event_counters.at(vcpu_index)++, cpu::SyscallAPI_t());
                };

                /**
                 * @brief Syscall return
                 *
                 * @param id The QEMU plugin ID
                 * @param vcpu_index The index of the VCPU
                 * @param num The syscall number
                 * @param ret The return value
                 *
                 * @return void
                 */
                static auto syscallReturn(qemu_plugin_id_t id, unsigned int vcpu_index,
                                          int64_t num, int64_t ret) -> void{};

                /**
                 * @brief Translate a basic block
                 *
                 * @param id The QEMU plugin ID
                 * @param tb The basic block to translate
                 *
                 * @return void
                 */
                static auto translateBasicBlock(qemu_plugin_id_t id, qemu_plugin_tb *tb) -> void
                {
                    qemu_plugin_insn *insn;
                    for (size_t i = 0; i < qemu_plugin_tb_n_insns(tb); ++i)
                    {
                        InstCacheEntry_t new_entry;
                        insn = qemu_plugin_tb_get_insn(tb, i);
                        new_entry.pc = qemu_plugin_insn_vaddr(insn);
                        new_entry.len = qemu_plugin_insn_size(insn);
                        switch (new_entry.len)
                        {
                        case 1:
                            new_entry.opcode = *((uint8_t *)qemu_plugin_insn_data(insn));
                            break;
                        case 2:
                            new_entry.opcode = *((uint16_t *)qemu_plugin_insn_data(insn));
                            break;
                        case 4:
                            new_entry.opcode = *((uint32_t *)qemu_plugin_insn_data(insn));
                            break;
                        default:
                            throw "Unknown instruction size!\n";
                            break;
                        }
                        // Insert into cache
                        {
                            // Acquire lock for cache
                            std::unique_lock<std::shared_mutex> lock(m_inst_cache_mutex);
                            // Insert into cache
                            m_inst_cache[new_entry.pc] = new_entry;
                        }
                        // Register instruction execution callback
                        qemu_plugin_register_vcpu_insn_exec_cb(insn, executeInstruction,
                                                               QEMU_PLUGIN_CB_NO_REGS, (void *)new_entry.pc);

                        // Register memory access callback
                        qemu_plugin_register_vcpu_mem_cb(insn, memoryAccess,
                                                         QEMU_PLUGIN_CB_NO_REGS, QEMU_PLUGIN_MEM_RW, NULL);
                    }
                };

                /**
                 * @brief Execute an instruction
                 *
                 * @param vcpu_index The index of the VCPU
                 * @param userdata The userdata pointer
                 *
                 * @return void
                 */
                static auto executeInstruction(unsigned int vcpu_index, void *userdata) -> void
                {
                    InstCacheEntry_t inst;
                    const Addr_t key = (Addr_t)userdata;
                    // Acquire lock for cache
                    {
                        std::shared_lock<std::shared_mutex> lock(m_inst_cache_mutex);
                        // Lookup in cache
                        inst = m_inst_cache[key];
                    }

                    // First time executing instruction
                    if(__glibc_likely(m_inst_counters.at(vcpu_index) != 0))
                    {
                        cpu::StaticInst_t &last_inst = m_last_insts.at(vcpu_index);
                        // Add next pc to last executed instruction
                        last_inst.br_info.target_pc = inst.pc;
                        if (last_inst.pc != inst.pc + inst.len)
                        {
                            last_inst.br_info.redirect = true;
                        }
                        // Send instruction
                        m_event_publishers.at(vcpu_index)->publish(
                            false, cpu::ThreadEvent_t::InsnTag, m_event_counters.at(vcpu_index)++, last_inst);
                    }
                    // Update last executed instruction
                    cpu::StaticInst_t &cur_inst = m_last_insts.at(vcpu_index);
                    cur_inst.uid = m_inst_counters.at(vcpu_index)++;
                    cur_inst.pc = inst.pc;
                    cur_inst.opcode = inst.opcode;
                    cur_inst.len = inst.len;
                };

                /**
                 * @brief Memory access
                 *
                 * @param vcpu_index The index of the VCPU
                 * @param info The memory access information
                 * @param vaddr The virtual address accessed
                 * @param userdata The userdata pointer
                 *
                 * @return void
                 */
                static auto memoryAccess(unsigned int vcpu_index, qemu_plugin_meminfo_t info,
                                         uint64_t vaddr, void *userdata) -> void
                {
                    cpu::StaticInst_t &cur_inst = m_last_insts.at(vcpu_index);
                    cur_inst.mem_info.vaddr = vaddr;
                    cur_inst.mem_info.len = qemu_plugin_mem_size_shift(info);
                    cur_inst.mem_info.is_store = qemu_plugin_mem_is_store(info);
                };

            public:
                // Application name
                static std::string m_app_name;
                // Runtime name
                static std::string m_runtime;
                // Process information
                static ProcessID_t m_pid;
                // Boot hart ID
                static HartID_t m_boot_hart;
                // Maximum number of harts
                static HartID_t m_max_harts;

            private:
                // Shared Resource Lock
                static std::mutex m_shared_resource_mutex;
                // Instruction counter for each VCPU
                static std::unordered_map<HartID_t, EventID_t> m_inst_counters;
                // Event counter for each VCPU
                static std::unordered_map<HartID_t, EventID_t> m_event_counters;
                // Last executed instructions for each VCPU
                static std::unordered_map<HartID_t, cpu::StaticInst_t> m_last_insts;
                // Event publisher
                static std::unordered_map<HartID_t, std::unique_ptr<EventPublisher>> m_event_publishers;
                // Instruction cache
                static InstCache_t m_inst_cache;
                // Mutex for instruction cache
                static std::shared_mutex m_inst_cache_mutex;
            };

        } // namespace qemu

    } // namespace iss

} // namespace archXplore
