#include <iss/qemu/qemuInterface.h>
#include <iostream>
#include <ctime>

using namespace archXplore::isa;
using namespace archXplore::iss::qemu;


void coreTimingThread(const std::shared_ptr<qemuInterface>& qemu_if, const size_t& cpu_index) {
    
    time_t start_time, end_time;
    double elapsed_second;
    
    traceInsn insn;
    bool exit_flag = false;
    uint64_t counter = 0;

    auto& insn_queue = qemu_if->getInsnQueueByIndex(cpu_index);

    start_time = clock();

    while(!exit_flag){
        // qemu_if->unblockQemuThread();
        insn_queue.pop(exit_flag,insn); 
        // std::cerr << "core" << cpu_index << ": uid " << std::dec << counter++ << " -> " <<
        // " pc " << std::hex << insn.pc <<
        // " opcode " << std::hex << insn.opcode << std::endl;
        counter++;
        // qemu_if->blockQemuThread();
    }

    end_time = clock();

    elapsed_second = double(end_time - start_time) / CLOCKS_PER_SEC;

    std::cerr << "Core " << cpu_index << ":" << std::endl
        << "\tTotal host time elapsed(s) : " << elapsed_second << std::endl
        << "\tTotal instruciton count : " << counter << std::endl
        << "\tMillion instructions per second(MIPS) : " << (((double)counter / 1000000.0) / elapsed_second)
        << std::endl;

}

std::thread create_timing_thread(const std::shared_ptr<qemuInterface>& qemu_if, const size_t& cpu_index){
    std::thread t(coreTimingThread,qemu_if,cpu_index);
    return t;
}

int main(int argc, char **argv, char **envp)
{

    time_t start_time, end_time;
    double elapsed_second;

    std::vector<std::thread> timing_thread_pool;

    std::shared_ptr<qemuInterface> qemu_if = qemuInterface::getInstance();

    auto& qemu_thread = qemu_if->createQemuThread({argc,argv,envp});

    for(size_t core_id = 0 ; core_id < 4  ; core_id++){
        timing_thread_pool.emplace_back(std::move(create_timing_thread(qemu_if,core_id)));
    }

    qemu_if->unblockQemuThread();

    for(auto it = timing_thread_pool.begin(); it != timing_thread_pool.end(); it++){
        if(it->joinable()){
            it->join();
        }
    }

    qemu_if->exit();

    return 0;
}
