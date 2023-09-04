#include <iss/qemu/qemuInterface.h>
#include <iostream>
#include <ctime>

using namespace archXplore::isa;
using namespace archXplore::iss::qemu;

int main(int argc, char **argv, char **envp)
{

    time_t start_time, end_time;
    double elapsed_second;

    std::shared_ptr<qemuInterface> qemu_if = qemuInterface::getInstance();
    auto& qemu_thread = qemu_if->createQemuThread({argc,argv,envp});
    auto& insn_queue_zero = qemu_if->getInsnQueueByIndex(0);
 
    bool exit_flag = false;
    uint64_t counter = 0;
    traceInsn insn;

    start_time = clock();
    qemu_if->unblockQemuThread();
    while(!exit_flag){
        insn_queue_zero.pop(exit_flag,insn); 
        // std::cerr << " uid " << std::dec << counter++ << " -> " <<
        // " pc " << std::hex << insn.pc <<
        // " opcode " << std::hex << insn.opcode << std::endl;
        counter++;
    }

    end_time = clock();

    elapsed_second = double(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(stderr, "Total host time elapsed(s) : %lf\
        \nTotal instruciton count : %ld\
        \nMillion instructions per second(MIPS) : %lf\n",
        elapsed_second,counter,(((double)counter / 1000000.0) / elapsed_second)
    );

    qemu_if->exit();

    return 0;
}
