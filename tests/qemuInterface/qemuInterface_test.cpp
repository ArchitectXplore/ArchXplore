#include <iss/qemu/qemuInterface.h>
#include <iostream>

using namespace archXplore::isa;
using namespace archXplore::iss::qemu;

std::shared_ptr<qemuInterface> qemu_if;

int main(int argc, char **argv, char **envp)
{

    qemu_if = qemuInterface::getInstance();
    auto& qemu_thread = qemu_if->createQemuThread({argc,argv,envp});
    auto& insn_queue_zero = qemu_if->getInsnQueueByIndex(0);
 
    bool exit_flag = false;
    uint64_t counter = 0;
    qemuInterface::insnPtr insn;

    while(!exit_flag){
        insn_queue_zero.pop(exit_flag,insn);
        // std::cout << " uid " << std::dec << counter++ << " -> " <<
        // " pc " << std::hex << insn->pc <<
        // " opcode " << std::hex << insn->opcode << std::endl;
    }
    return 0;
}
