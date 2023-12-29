#pragma once

extern "C" {
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


namespace archXplore {
namespace iss {
namespace qemu {


class qemuInterface;

// QEMU Interface Instance variable
extern std::mutex g_qemu_if_lock;
extern std::shared_ptr<qemuInterface> g_qemu_if;
// QEMU Thread
extern std::unique_ptr<std::thread> g_qemu_thread;

struct qemuArgs_t {
    int argc;
    char** argv;
    char** envp;
};

// class CPUState {
// public:
//     CPUState() {};
//     ~CPUState() {};
//     auto popHead() -> isa::traceInsn {
//     };
//     auto exit() -> void {
//     };
// private:
//     std::mutex m_lock;
//     std::condition_variable m_cond;
//     insnTunnel<isa::traceInsn> m_insnTunnel;
// };




class qemuInterface 
{
private:
    
    template <typename T>
    struct userDataMaintainer {
    public:
        void append(T* data) {
            m_dataVec.push_back(data);
        };
        void flush() { 
            do { 
                delete m_dataVec.front();
                m_dataVec.pop_front();
            } while(m_dataVec.size());
        }
    private:
        std::vector<T*> m_dataVec;
    };

public:
    using ptrType = std::shared_ptr<qemuInterface>;
    // Deleted function 
    qemuInterface(qemuInterface&) = delete;
    qemuInterface(const qemuInterface&) = delete;
    qemuInterface& operator=(const qemuInterface&) = delete;
    // Default constructor 
    qemuInterface() {};
    // Deconstructor
    ~qemuInterface() {}; 
    // Entry function to start qemu thread
    auto bootQemuThread(const qemuArgs_t& args) -> void {
        std::lock_guard<std::mutex> lock(g_qemu_if_lock);
        if(m_qemu_thread == nullptr) {
            #ifndef CONFIG_USER_ONLY
                m_qemu_thread = std::make_unique<std::thread>(qemuSystemEmulator, args.argc, args.argv);
            #else 
                m_qemu_thread = std::make_unique<std::thread>(qemuUserEmulator, args.argc, args.argv, args.envp);
            #endif
        };
    };
    // Qemu thread guard
    auto qemuThreadJoin() -> void { 
        if(m_qemu_thread->joinable()){
            m_qemu_thread->join();
        }
    };
    // Block QEMU thread
    auto blockQemuThread() -> void { 
        // Lock QEMU IO thread
        qemu_plugin_mutex_lock_iothread();
    };
    // Unblock QEMU thread
    auto unblockQemuThread() -> void { 
        // Unlock QEMU IO thread
        qemu_plugin_mutex_unlock_iothread();
    };
    static auto fetchAddEventId() -> eventId_t { 
        return m_qemu_sync_event_id.fetch_add(1);
    };
    static auto pendingSyncEvent() -> bool {
        return m_qemu_sync_event_pending.load(std::memory_order::memory_order_seq_cst);
    };
    static auto addSyncEvent(const systemSyncEvent_t& ev) -> void { 
        // Get Unique Lock to block qemu vCPU thread
        {
            std::unique_lock<std::shared_mutex> sync_lock(m_qemu_sync_lock);
            // Get Unique lock to write qemu-related structure
            {
                std::unique_lock<std::mutex> lock(m_qemu_lock);
                // Push sync event
                m_qemu_sync_event_pending.store(true, std::memory_order::memory_order_seq_cst);
                m_qemu_sync_event = ev;
                // Wait for processing and wakeup
                while(pendingSyncEvent()) {
                    m_qemu_cond.wait(lock);
                };
            }
        }
    };
    static auto getPendingSyncEvent() -> systemSyncEvent_t {
        return m_qemu_sync_event;
    };
    static auto removeSyncEvent() -> void {
        // Get Unique lock to write qemu-related structure
        {
            std::unique_lock<std::mutex> lock(m_qemu_lock);
            m_qemu_sync_event_pending.store(false, std::memory_order::memory_order_seq_cst);
            // Wait up sync event owner
            m_qemu_cond.notify_one();
        }
    };
    // The only entry to get/construct qemuInterface
    static auto getInstance() -> std::shared_ptr<qemuInterface> {
        std::lock_guard<std::mutex> lock(g_qemu_if_lock);
        if(g_qemu_if == nullptr) {
            g_qemu_if = std::make_shared<qemuInterface>();
        }
        return g_qemu_if;
    };
    /* Instrument functions for QEMU */
    static auto qemu_vcpu_init(qemu_plugin_id_t id, unsigned int vcpu_index) -> void {
        // Add CPU Init Event
        systemSyncEvent_t ev;
        ev.event_id = fetchAddEventId();
        ev.event_type = systemSyncEventTypeEnum_t::hartInit;
        ev.hart_id = vcpu_index;
        addSyncEvent(ev);
    };
    static auto qemu_vcpu_exit(qemu_plugin_id_t id, unsigned int vcpu_index) -> void {
        
    };
    static auto qemu_exit(qemu_plugin_id_t id, void *userdata) -> void { 
        // Add QEMU Exit Event
        systemSyncEvent_t ev;
        ev.event_id = fetchAddEventId();
        ev.event_type = systemSyncEventTypeEnum_t::systemExit;
        addSyncEvent(ev);
    };
    static auto qemu_vcpu_tb_trans(qemu_plugin_id_t id, qemu_plugin_tb *tb) -> void { 
        
        qemu_plugin_insn* insn;
        
        for(size_t i = 0 ; i < qemu_plugin_tb_n_insns(tb); ++i) {

            insn = qemu_plugin_tb_get_insn(tb,i);

            // std::cerr << "Translate Insn Address " << std::hex << qemu_plugin_insn_vaddr(insn) << std::endl;

        } 
    
    };

private:
    // QEMU Thread
    static std::unique_ptr<std::thread> m_qemu_thread;
    // QEMU operation mutex
    static std::mutex m_qemu_lock;
    static std::shared_mutex m_qemu_sync_lock;
    static std::condition_variable m_qemu_cond;
    // QEMU status
    static std::atomic<eventId_t> m_qemu_sync_event_id;
    // Global Sync Event Register
    static std::atomic<bool> m_qemu_sync_event_pending;
    static systemSyncEvent_t m_qemu_sync_event;
};
    // QEMU Thread
    std::unique_ptr<std::thread> qemuInterface::m_qemu_thread = nullptr;
    // QEMU operation mutex
    std::mutex qemuInterface::m_qemu_lock;
    std::shared_mutex qemuInterface::m_qemu_sync_lock;
    std::condition_variable qemuInterface::m_qemu_cond;
    // QEMU status
    std::atomic<eventId_t> qemuInterface::m_qemu_sync_event_id = 0;
    // Global Sync Event Register
    std::atomic<bool> qemuInterface::m_qemu_sync_event_pending = false;
    systemSyncEvent_t qemuInterface::m_qemu_sync_event;
}
}
}