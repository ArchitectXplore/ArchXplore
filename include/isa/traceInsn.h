#pragma once 

#include <memory>

namespace archXplore
{
namespace isa
{

struct traceInsn {
    typedef std::shared_ptr<traceInsn> PtrType;
    uint64_t pc;
    uint32_t opcode;
};
struct traceMemInsn : public traceInsn {
    uint64_t addr;
    bool io_device;
};
struct sysTraceInsn : public traceInsn {
    uint64_t physical_pc;
};
struct sysTraceMemInsn : public sysTraceInsn, public traceMemInsn {
    uint64_t physical_addr;
};

}

}