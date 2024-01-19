#pragma once 

#include <iostream>
#include <ostream>
#include <iomanip>
#include <string>
#include <memory>

#include "iss/type.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"
#include "sparta/utils/SpartaSharedPointerAllocator.hpp"

namespace archXplore {
namespace isa {

struct memAccess_t {
    iss::addr_t vaddr;
    // iss::addr_t paddr;
    uint8_t len;
};

struct traceInsn_t {
    // Unique instruction id    
    iss::eventId_t uid;
    // instruction information
    iss::addr_t pc;
    // iss::addr_t pc_paddr;
    iss::opcode_t opcode;
    uint8_t len;    
    // target pc
    iss::addr_t target_pc;
    // Memory Access
    std::vector<memAccess_t> mem;

    std::string stringize() const {
        std::stringstream ss;
        ss << std::dec << uid << " -> " << std::hex << pc; 
        return ss.str();
    };
};

using traceInsnPtr_t = sparta::SpartaSharedPointer<traceInsn_t>;

}
}

inline std::ostream& operator<< (std::ostream& out, archXplore::isa::traceInsn_t const & ps){
    out << ps.stringize();
    return out;
}

inline std::ostream& operator<< (std::ostream& out, archXplore::isa::traceInsn_t const * ps){
    out << ps->stringize();
    return out;
}
