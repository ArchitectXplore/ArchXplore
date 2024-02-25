#include "iss/qemu/qemuInterface.hpp"



using namespace archXplore::iss::qemu;

int main(int argc, char **argv)
{
    /* code */
    qemuInterface* qemu_if = qemuInterface::getInstance().get();

    qemu_if->bootQemuThread({argc, argv});

    auto event_queue_ptr = qemu_if->getHartEventQueuePtr(0);

    m_simulated_cpu_number = 1;

    // Instruction counter
    uint64_t counter = 0;

    // Profiling loop
    auto start_time = std::chrono::high_resolution_clock::now();

    while(true) {
        auto event = event_queue_ptr->front();
        if(event.tag == event.InsnTag) {
            counter++;
            if(event.instruction.is_last) {
                break;
            }
        } else {
            continue;
        }
        event_queue_ptr->pop();
    }   

    auto end_time = std::chrono::high_resolution_clock::now();

    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::cout << "Instructions executed: " << counter << std::endl;
    std::cout << "Execution time: " << elapsed_time << " microseconds" << std::endl;
    std::cout << "Instructions per second: " << (counter / (elapsed_time / 1000000.0)) << std::endl;

    qemu_if->qemu_shutdown(0);

    return 0;
}
