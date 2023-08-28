#pragma once
extern "C"{
    #include <iss/qemu/qemuEmulator.h>
} 
#include <iss/insnTunnel.h>
#include <isa/traceInsn.h>
#include <thread>
#include <mutex>
#include <cmath>

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
    ~qemuInterface(){mainExit();};

    void mainExit(){
        std::unique_lock<std::mutex> lock(m_exit_lock);
        exit_ready = true;
        m_exit_cond.notify_one();
    };

    void qemuExitRequest() {
        for(auto it = m_insn_queue.begin(); it != m_insn_queue.end(); it++){
            it->producer_do_exit();
        }
        std::unique_lock<std::mutex> lock(m_exit_lock);
        while(!exit_ready){
            m_exit_cond.wait(lock);
        }
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
                    m_insn_queue.emplace_back(insnTunnel<insnPtr>(m_tunnleNumber,m_simInterval));
                }
            }
            m_insn_queue_lock.unlock();
        }
    };

    void set_sim_interval(const size_t& simInterval, const size_t& tunnelNumber = 1){
        m_simInterval = ((size_t)std::ceil(((double)simInterval/8.0))) * 8; // Align to 64 bytes
        m_tunnleNumber = tunnelNumber;
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
        : m_simInterval(16384), m_tunnleNumber(1), exit_ready(false)
    {};
private:
    size_t m_simInterval;
    size_t m_tunnleNumber;

    std::vector<insnTunnel<insnPtr>> m_insn_queue;
    std::mutex m_insn_queue_lock;

    bool exit_ready;
    std::mutex m_exit_lock;
    std::condition_variable m_exit_cond;
};

} // namespace qemu
} // namespace iss
} // namespace archXplore
