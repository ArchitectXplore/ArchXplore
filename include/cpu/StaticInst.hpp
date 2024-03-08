#pragma once

#include <iomanip>

#include "Types.hpp"

namespace archXplore
{
    namespace cpu
    {
        enum FunctionInfo_t
        {
            TYPE_UNKNOWN,
            TYPE_ALU,
            TYPE_LOAD,
            TYPE_STORE,
            TYPE_BRANCH,
            TYPE_JUMP,
            TYPE_JUMP_REG,
            TYPE_CALL,
            TYPE_RETURN,
            TYPE_SYSTEM,
            TYPE_OTHER
        };

        enum RegType_t
        {
            REG_TYPE_NONE,
            REG_TYPE_GPR,
            REG_TYPE_FPR,
            REG_TYPE_VPR
        };

        struct RegisterInfo_t
        {
            RegType_t type;
            uint8_t id;
        };

        struct MemoryInfo_t
        {
            Addr_t vaddr;
            uint8_t len;
            bool is_store;
        };

        struct BranchInfo_t
        {
            bool redirect;
            Addr_t target_pc;
        };

        struct StaticInst_t
        {
            // Instruction ID
            EventID_t uid;
            // Instruction information
            Addr_t pc;
            // Opcode
            uint32_t opcode;
            // Instruction length
            uint8_t len;
            // Instruction source registers
            RegisterInfo_t src_reg1;
            RegisterInfo_t src_reg2;
            RegisterInfo_t src_reg3;
            // Instruction destination register
            RegisterInfo_t dst_reg;
            // Instruction function information
            FunctionInfo_t func_info;
            // Branch information
            BranchInfo_t br_info;
            // Memory accesses
            MemoryInfo_t mem_info;

            inline std::string serialize() const
            {
                std::stringstream ss;
                ss << std::dec << uid << " -> " << std::hex << pc;
                return ss.str();
            };
        };
    } // namespace cpu

} // namespace archXplore