extern "C" {
#include <iss/qemu/qemu-plugin.h>
}

#include <isa/traceInsn.h>
#include <iss/qemu/qemuInterface.h>

#include <vector>
#include <mutex>

using namespace archXplore::iss::qemu;
using namespace archXplore::isa;

static bool system_emulation;

std::shared_ptr<qemuInterface> interface_instance;

static std::vector<qemuInterface::insnPtr> last_exec_insn;

static std::mutex last_exec_insn_lock;

static void resize_last_exec_insn(const size_t& coreNumber) {
    if(coreNumber > last_exec_insn.size()){
        std::lock_guard<std::mutex> lock(last_exec_insn_lock);
        if(coreNumber > last_exec_insn.size()){
            last_exec_insn.resize(coreNumber);
            interface_instance->resize_insn_tunnel(coreNumber);
        }
    }
};

static void send_insn_by_hart(const size_t& cpu_index){
    auto& insn_queue = interface_instance->getInsnQueueByIndex(cpu_index);
    auto& insn = last_exec_insn[cpu_index];
    if(insn){
        insn_queue.push(insn);
        insn.reset();
    }
};

static void vcpu_mem(unsigned int cpu_index, qemu_plugin_meminfo_t info,
                     uint64_t vaddr, void *udata)
{
    auto& insn = last_exec_insn[cpu_index];
    insn->addr = vaddr;
    if(system_emulation) {
        qemu_plugin_hwaddr* mem_hwaddr = (qemu_plugin_hwaddr*) qemu_plugin_get_hwaddr(info,vaddr);
        insn->physical_addr = qemu_plugin_hwaddr_phys_addr(mem_hwaddr);
    } else {
        insn->physical_addr = vaddr;
    }
}

static void vcpu_insn_exec(unsigned int cpu_index, void *udata)
{
    resize_last_exec_insn(cpu_index+1);

    auto& last_insn_ptr = last_exec_insn[cpu_index];

    send_insn_by_hart(cpu_index);

    last_insn_ptr.reset((traceInsn*) udata);
}

static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    
    qemu_plugin_insn* insn; 

    for (size_t i = 0; i < qemu_plugin_tb_n_insns(tb); i++) {

        traceInsn* trace_insn = new traceInsn(); 

        insn = qemu_plugin_tb_get_insn(tb,i);

        trace_insn->pc = qemu_plugin_insn_vaddr(insn);
        
        if(system_emulation) {
            qemu_plugin_hwaddr* pc_hwaddr = (qemu_plugin_hwaddr*) qemu_plugin_insn_haddr(insn);
            if(qemu_plugin_hwaddr_is_io(pc_hwaddr)){
                trace_insn->physical_pc = trace_insn->pc;
            } else {
                trace_insn->physical_pc = qemu_plugin_hwaddr_phys_addr(pc_hwaddr);
            }
        } else {
            trace_insn->physical_pc = trace_insn->pc;
        }

        trace_insn->opcode = *((uint32_t *)qemu_plugin_insn_data(insn));

        if( qemu_plugin_insn_size(insn) == 2){
            trace_insn->opcode = trace_insn->opcode & 0xFFFF;
        }

        /* Register callback on memory read or write */
        qemu_plugin_register_vcpu_mem_cb(insn, vcpu_mem,
                                            QEMU_PLUGIN_CB_NO_REGS,
                                            QEMU_PLUGIN_MEM_RW, NULL);

        /* Register callback on instruction */
        qemu_plugin_register_vcpu_insn_exec_cb(insn, vcpu_insn_exec,
                                                QEMU_PLUGIN_CB_NO_REGS, (void*) trace_insn);

    }

}


static void plugin_init(void)
{
    interface_instance = qemuInterface::getInstance();
}


static void plugin_exit(qemu_plugin_id_t id, void *p)
{
    for(size_t i = 0 ; i < last_exec_insn.size(); i++){
        send_insn_by_hart(i);
    }
    interface_instance->qemuExitRequest();
}

extern "C" {

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

QEMU_PLUGIN_EXPORT
int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info,
                        int argc, char **argv)
{

    system_emulation = info->system_emulation; 

    plugin_init();
    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);
    qemu_plugin_register_atexit_cb(id, plugin_exit, NULL);
    return 0;
}

}