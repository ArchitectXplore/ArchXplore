#pragma once
extern "C"{
    #include <iss/qemu/qemuEmulator.h>
} 
#include <iss/insnTunnel.h>
#include <isa/traceInsn.h>
#include <thread>
#include <mutex>
#include <iostream>

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
public:
    typedef std::shared_ptr<isa::traceInsn> insnPtr;

public:
    // Delected function
    qemuInterface(qemuInterface&) = delete;
    qemuInterface& operator=(const qemuInterface&) = delete;

public:
    ~qemuInterface(){
        m_exit_lock.unlock();
    };

    void qemuExitRequest() {
        for(auto it = m_insn_queue.begin(); it != m_insn_queue.end(); it++){
            it->producer_do_exit();
        }
        m_exit_lock.lock();
    };

    insnTunnel<insnPtr>& getInsnQueueByIndex(const uint64_t& hart_index){
        resize_insn_tunnel(hart_index+1);
        return m_insn_queue[hart_index];
    };

    void resize_insn_tunnel(const size_t& coreNumber){
        if(coreNumber > m_insn_queue.size()) {
            m_insn_queue_lock.lock();
            if(coreNumber > m_insn_queue.size()) {
                while(coreNumber > m_insn_queue.size()){
                    m_insn_queue.emplace_back(insnTunnel<insnPtr>(1,m_simInterval));
                }
            }
            m_insn_queue_lock.unlock();
        }
    };

    void set_sim_interval(const size_t& simInterval){
        m_simInterval = simInterval;
    };

    static std::shared_ptr<qemuInterface>& getInstance(){
        if(g_qemuInterface_instance == nullptr){
            g_qemuInterface_lock.lock();
            if(g_qemuInterface_instance == nullptr) {
                g_qemuInterface_instance = std::shared_ptr<qemuInterface>(new qemuInterface);
            }
            g_qemuInterface_lock.unlock();
        }
        return g_qemuInterface_instance;
    };

    std::thread& createQemuThread(const qemuArgument_t& qemu_args) {
        m_exit_lock.lock();
        #ifndef CONFIG_USER_ONLY
        static std::thread t(qemuSystemEmulator, qemu_args.argc, qemu_args.argv);
        #else
        static std::thread t(qemuUserEmulator, qemu_args.argc, qemu_args.argv, qemu_args.envp);
        #endif
        t.detach();
        return t;
    };

private:
    qemuInterface() 
        : m_simInterval(10000)
    {
        resize_insn_tunnel(1);
    };
    qemuInterface(const size_t& coreNumber, const size_t& simInterval) 
        : m_simInterval(simInterval)
    {
        resize_insn_tunnel(coreNumber);
    };
private:
    size_t m_simInterval;

    std::vector<insnTunnel<insnPtr>> m_insn_queue;
    std::mutex m_insn_queue_lock;

    std::mutex m_exit_lock;
};

} // namespace qemu
} // namespace iss
} // namespace archXplore
