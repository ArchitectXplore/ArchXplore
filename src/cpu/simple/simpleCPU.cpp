#include "cpu/simple/simpleCPU.hpp"

#include "python/embeddedModule.hpp"

namespace archXplore
{
    namespace cpu
    {
        namespace simple
        {

            const char simpleCPU::name[] = "simpleCPU";

            simpleCPU::simpleCPU(sparta::TreeNode *tn, const simpleCPUParams *params)
                : abstractCPU(tn), m_params(params){};

            simpleCPU::~simpleCPU(){};

            auto simpleCPU::reset() -> void
            {
                m_next_pc = m_boot_pc;
            };

            auto simpleCPU::tick() -> void
            {
                m_iss->generateFetchRequest(m_next_pc, m_params->fetch_width);
                // 1. Fetch instructions
                auto inst_group = m_iss->processFetchResponse(nullptr);
                // 2. Push instructions into instruction buffer
                for (auto &inst : inst_group)
                {
                    m_inst_buffer.emplace(std::make_shared<instruction_t>(inst));
                }
                // 3. Execute instructions
                if (!m_inst_buffer.empty())
                {
                    auto inst = m_inst_buffer.front();
                    if (SPARTA_EXPECT_FALSE(debug_logger_))
                    {
                        debug_logger_ << "Execute Instruction -> "
                                      << "uid[" << std::dec << inst->uid << "], "
                                      << "pc[" << std::hex << inst->pc << "], "
                                      << "opcode[" << std::hex << inst->opcode << "]" << std::endl;
                    }
                    m_instret++;
                    m_inst_buffer.pop();
                    // 4. Update Next PC
                    if (inst->br_info.redirect)
                    {
                        m_next_pc = inst->br_info.target_pc;
                    }
                    else
                    {
                        m_next_pc = inst->pc + inst->len;
                    }

                    if (SPARTA_EXPECT_FALSE(inst->is_last && info_logger_))
                    {
                        info_logger_ << m_cycle;
                        info_logger_ << m_instret;
                    }
                }
            };

            REGISTER_SPARTA_UNIT(simpleCPU, simpleCPUParams);

        }
    }
}
