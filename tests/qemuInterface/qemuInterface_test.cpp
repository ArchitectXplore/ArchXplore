#include <iostream>
#include <ctime>
#include "iss/qemu/qemuInterface.hpp"

using namespace archXplore::isa;
using namespace archXplore::iss;
using namespace archXplore::iss::qemu;


int main(int argc, char **argv, char **envp)
{

    time_t start_time, end_time;
    double elapsed_second;

    qemuInterface::ptrType qemu_if = qemuInterface::getInstance();

    qemu_if->bootQemuThread({argc, argv, envp});

    bool exit_flag = false;

    bool hart_init = false;

    while(!exit_flag) { 
        if(qemu_if->pendingSyncEvent()) { 
            // Drain 
            if(hart_init) {
                while(!qemu_if->getHartInsnQueuePtr(0)->isEmpty()){
                    qemu_if->getHartInsnQueuePtr(0)->pop();
                }
            }
            auto ev  = qemu_if->getPendingSyncEvent();
            if(ev.event_type == systemExit) {
                std::cerr << "QEMU Exit " << std::endl;
                exit_flag = true;
            }
            else if(ev.event_type == hartInit) {
                std::cerr << "Init hart 0 " << std::endl;
                hart_init = true;
            }
            qemu_if->removeSyncEvent();
        } else {
            if(hart_init) qemu_if->getHartInsnQueuePtr(0)->pop();
        }
    }

    qemu_if->qemuThreadJoin();
    std::cerr << "QEMU Join " << std::endl;

    return 0;
}
