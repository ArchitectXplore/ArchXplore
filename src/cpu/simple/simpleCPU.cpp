#include "cpu/simple/simpleCPU.hpp"
#include "system/abstractSystem.hpp"
#include "python/embeddedModule.hpp"

namespace archXplore
{
    namespace cpu
    {
        namespace simple
        {

            const char *simpleCPU::name = "simpleCPU";

            simpleCPU::simpleCPU(sparta::TreeNode *tn, const simpleCPUParams *params)
                : abstractCPU(tn, params->tid, params->frequency),
                  m_insn_exec_event(&unit_event_set_, "InsnExecutionEvent",
                                    CREATE_SPARTA_HANDLER(simpleCPU, exec)),
                  m_cycle(&unit_stat_set_, "cycle", "CPU runtime in cycle",
                          sparta::CounterBase::CounterBehavior::COUNT_NORMAL),
                  m_instret(&unit_stat_set_, "instret", "Counter of retired instructions",
                            sparta::CounterBase::CounterBehavior::COUNT_NORMAL){};

            simpleCPU::~simpleCPU(){};

            auto simpleCPU::reset() -> void
            {
                exec();
            };

            auto simpleCPU::cleanUp() -> void
            {
                info_logger_ << m_cycle;
                info_logger_ << m_instret;
            };

            auto simpleCPU::exec() -> void
            {
                if (isRunning())
                {
                    isa::traceInsnPtr_t insn;
                    auto iss = getISSPtr();
                    iss->receiveInstruction(insn);
                    debug_logger_ << "Execute Instruction -> "
                                  << "uid[" << std::dec << insn->uid << "], "
                                  << "pc[" << std::hex << insn->pc << "], "
                                  << "opcode[" << std::hex << insn->opcode << "]" << std::endl;
                    m_inst_retired++;
                    m_cycle++;
                    m_insn_exec_event.schedule(1);
                }
            };

            REGISTER_SPARTA_UNIT(simpleCPU, simpleCPUParams);

        }
    }
}
