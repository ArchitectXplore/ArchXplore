#include "cpu/simple/SimpleCPU.hpp"

#include "python/EmbeddedModule.hpp"

namespace archXplore
{
    namespace cpu
    {
        namespace simple
        {

            const char SimpleCPU::name[] = "SimpleCPU";

            SimpleCPU::SimpleCPU(sparta::TreeNode *tn, const SimpleCPUParams *params)
                : AbstractCPU(tn), m_params(params)
                // histogram_test(this->getStatisticSet(),"histogramTest", "Histogram Test", 1, 10, 2)
                {
                    // histogram_test.addValue(1);
                };

            SimpleCPU::~SimpleCPU(){};

            auto SimpleCPU::reset() -> void
            {
                m_next_pc = m_boot_pc;
            };

            auto SimpleCPU::tick() -> void
            {
                m_iss->generateFetchRequest(m_next_pc, m_params->fetch_width);
                // 1. Fetch instructions
                auto inst_group = m_iss->processFetchResponse(nullptr);
                // 2. Push instructions into instruction buffer
                for (auto &inst : inst_group)
                {
                    m_inst_buffer.emplace(std::make_shared<StaticInst_t>(inst));
                }
                // 3. Execute instructions
                if (!m_inst_buffer.empty())
                {
                    auto inst = m_inst_buffer.front();
                    if (SPARTA_EXPECT_FALSE(m_trace_logger))
                    {
                        m_trace_logger << "Execute Instruction -> "
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
                }
            };

            REGISTER_SPARTA_UNIT(SimpleCPU, SimpleCPUParams);

        }
    }
}
