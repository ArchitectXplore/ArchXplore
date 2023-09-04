#pragma once
extern "C"{
    #include <iss/qemu/qemuEmulator.h>
    #include <iss/qemu/qemu-plugin.h>
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
    ~qemuInterface(){};

    void exit(){
        std::unique_lock<std::mutex> lock(m_exit_lock);
        m_exit_ready = true;
        m_exit_cond.notify_one();
    };

    void qemuExitRequest() {
        for(auto it = m_insn_queue.begin(); it != m_insn_queue.end(); it++){
            it->producer_do_exit();
        }
        std::unique_lock<std::mutex> lock(m_exit_lock);
        m_exit_pending = true;
        while(!m_exit_ready){
            m_exit_cond.wait(lock);
        }
    };

    void qemuInitDone(){
        std::unique_lock<std::mutex> lock(m_init_lock);
        init_done = true;
        m_init_cond.notify_one();
    };

    void waitQemuInit(){
        std::unique_lock<std::mutex> lock(m_init_lock);
        while(!init_done){
            m_init_cond.wait(lock);
        }
        blockQemuThread();
    };

    insnTunnel<isa::traceInsn>& getInsnQueueByIndex(const uint64_t& hart_index){
        resizeInsnTunnel(hart_index+1);
        return m_insn_queue[hart_index];
    };

    void resizeInsnTunnel(const size_t& coreNumber){
        if(coreNumber > m_insn_queue.size()) {
            std::lock_guard<std::mutex> lock(m_insn_queue_lock);
            if(coreNumber > m_insn_queue.size()) {
                while(coreNumber > m_insn_queue.size()){
                    m_insn_queue.emplace_back(std::move(insnTunnel<isa::traceInsn>(m_tunnleNumber,m_simInterval)));
                }
            }
        }
    };

    void setSimInterval(const size_t& simInterval, const size_t& tunnelNumber = 1){
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
        waitQemuInit();
        t.detach();
        return t;
    };

    void blockQemuThread(){
        std::unique_lock<std::mutex> lock(m_exit_lock);
        if(!m_exit_pending){
            plugin_qemu_mutex_lock_iothread();
        }
    };

    void unblockQemuThread(){
        std::unique_lock<std::mutex> lock(m_exit_lock);
        if(!m_exit_pending){
            plugin_qemu_mutex_unlock_iothread();
        }
    };

private:
    qemuInterface() 
        : m_simInterval(16384), m_tunnleNumber(1), m_exit_ready(false), init_done(false), m_exit_pending(false)
    {
    };
private:
    size_t m_simInterval;
    size_t m_tunnleNumber;

    std::deque<insnTunnel<isa::traceInsn>> m_insn_queue;
    std::mutex m_insn_queue_lock;

    bool init_done;
    std::mutex m_init_lock;
    std::condition_variable m_init_cond;

    bool m_exit_pending;
    bool m_exit_ready;
    std::mutex m_exit_lock;
    std::condition_variable m_exit_cond;
};

} // namespace qemu
} // namespace iss
} // namespace archXplore
