#include "uncore/memory/dramsim3_wrapper.hpp"
#include "utils/common.hpp"
#include <cinttypes>
#include <iostream>
#include <string>
using namespace archXplore::uncore;
auto readCB(uint64_t data) -> void{
    std::cout << "Read data: " << HEX16(data) << std::endl;
}
auto writeCB(uint64_t data) -> void{
    std::cout << "Write data: " << HEX16(data) << std::endl;
}
std::string config_file = "/root/ArchXplore/ext/DRAMsim3/configs/DDR3_4Gb_x4_1600.ini";
std::string working_dir = "/root/ArchXplore/tests/uncoreTest";
int main(){
    uint64_t addr = 0x0;
    DRAMSim3Wrapper dram(config_file, working_dir, readCB, writeCB);
    std::cout << "burst size" << dram.burstSize() << std::endl;
    std::cout << "queue size" << dram.queueSize() << std::endl;
    std::cout << "can accept" << dram.canAccept(addr, false) << std::endl;
    std::cout << "clock period" << dram.clockPeriod() << std::endl;

    dram.tick();
    dram.tick();
    dram.enqueue(0x1234, false);
    for(int i = 0; i < 1000; i++){
        // std::cout << "can accept" << dram.canAccept(addr, false) << std::endl;
        // std::cout << "tick" << std::endl;
        dram.tick();
    }
    return 0;
}