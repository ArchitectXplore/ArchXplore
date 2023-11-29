#pragma once 

#include <memory>

namespace archXplore
{
namespace isa
{

struct traceInsn {
    uint64_t pc;
    uint32_t opcode;
    
    uint64_t addr;
    bool io_device;

    uint64_t physical_pc;
    uint64_t physical_addr;
};
}

}