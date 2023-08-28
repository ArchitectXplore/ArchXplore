#include <iss/qemu/qemuInterface.h>
#include <iostream>
#include <ctime>

using namespace archXplore::isa;
using namespace archXplore::iss::qemu;

std::shared_ptr<qemuInterface> qemu_if;

int main(int argc, char **argv, char **envp)
{

    time_t start_time, end_time;
    double elapsed_second;

    qemu_if = qemuInterface::getInstance();
    auto& qemu_thread = qemu_if->createQemuThread({argc,argv,envp});
    auto& insn_queue_zero = qemu_if->getInsnQueueByIndex(0);
 
    bool exit_flag = false;
    uint64_t counter = 0;
    qemuInterface::insnPtr insn;

    start_time = clock();

    while(!exit_flag){
        insn_queue_zero.pop(exit_flag,insn);
        // std::cerr << " uid " << std::dec << counter++ << " -> " <<
        // " pc " << std::hex << insn->pc <<
        // " opcode " << std::hex << insn->opcode << std::endl;
        counter++;
    }

    end_time = clock();

    elapsed_second = double(end_time - start_time) / CLOCKS_PER_SEC;

    std::cerr << "Total host time elapsed(s) : " << elapsed_second << std::endl
        << "Total instruciton count : " << counter << std::endl
        << "Million instructions per second(MIPS) : " << (((double)counter / 1000000.0) / elapsed_second)
        << std::endl;

    return 0;
}
