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

    while(!exit_flag) { 
        if(qemu_if->pendingSyncEvent()) { 
            auto ev = qemu_if->getPendingSyncEvent();
            if(ev.event_type == systemExit) {
                exit_flag = true;
            }
            else if(ev.event_type == hartInit) {
            }
            qemu_if->removeSyncEvent();
        }
    }

    qemu_if->qemuThreadJoin();

    return 0;
}
