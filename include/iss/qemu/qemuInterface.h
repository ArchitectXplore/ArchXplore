#pragma once

#include <iss/qemu/qemuEmulator.h>
#include <iss/insnTunnel.h>
#include <isa/traceInsn.h>
#include <thread>
#include <mutex>

namespace archXplore {
namespace iss {
namespace qemu {

class qemuInterface;

extern std::shared_ptr<qemuInterface> g_qemuInterface_instance;
extern std::mutex g_qemuInterface_lock;

struct qemuArgument_t {
    int argc;
    char **argv;
    char **envp;
};

class qemuInterface
{
private:
    using insnPtr = isa::traceInsn::PtrType;

public:
    // Delected function
    qemuInterface(qemuInterface&) = delete;
    qemuInterface& operator=(const qemuInterface&) = delete;

public:
    ~qemuInterface(){};

    void qemuExitRequest() {
        for(auto it = m_insn_queue.begin(); it != m_insn_queue.end(); it++){
            it->producer_do_exit();
        }
    };

    insnTunnel<insnPtr>& getInsnQueueByIndex(const uint64_t& hart_index){
        resize_insn_tunnel(hart_index+1);
        return m_insn_queue[hart_index];
    };

    void resize_insn_tunnel(const size_t& coreNumber){
        if(coreNumber > m_coreNumber) {
            m_insn_queue_lock.lock();
            if(coreNumber > m_coreNumber) {
                while(coreNumber > m_coreNumber){
                    m_insn_queue.emplace_back(insnTunnel(2,m_simInterval / 2));
                }
                m_coreNumber = coreNumber;
            }
            m_insn_queue_lock.unlock();
        }
    };

    static std::shared_ptr<qemuInterface>& getInstance(const size_t coreNumber, const size_t simInterval){
        if(g_qemuInterface_instance == nullptr){
            g_qemuInterface_lock.lock();
            if(g_qemuInterface_instance == nullptr) {
                g_qemuInterface_instance = std::shared_ptr<qemuInterface>(new qemuInterface(coreNumber, simInterval));
            }
            g_qemuInterface_lock.unlock();
        }
        return g_qemuInterface_instance;
    };

    std::thread& createQemuThread(const bool& user_mode, const qemuArgument_t& qemu_args) {
        m_exit_lock.lock();
        if(user_mode){
            static std::thread t(qemu_args.argc, qemu_args.argv, qemu_args.envp);
            t.detach();
            return t;
        } else {
            static std::thread t(qemu_args.argc, qemu_args.argv);
            t.detach();
            return t;
        }
    };

private:
    qemuInterface(const size_t& coreNumber, const size_t& simInterval) 
        :  m_coreNumber(coreNumber), m_simInterval(simInterval)
    {
        resize_insn_tunnel(coreNumber);
    };
private:
    size_t m_coreNumber;
    size_t m_simInterval;

    std::vector<insnTunnel<insnPtr>> m_insn_queue;
    std::mutex m_insn_queue_lock;

    std::mutex m_exit_lock;
};

} // namespace qemu
} // namespace iss
} // namespace archXplore
