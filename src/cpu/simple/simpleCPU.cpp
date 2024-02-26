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
                : abstractCPU(tn),
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

            auto simpleCPU::exec() -> void
            {
                auto iss = getISSPtr();
                auto insn = iss->generateFetchRequest();
                if (debug_logger_)
                {
                    debug_logger_ << "Execute Instruction -> "
                                  << "uid[" << std::dec << insn->uid << "], "
                                  << "pc[" << std::hex << insn->pc << "], "
                                  << "opcode[" << std::hex << insn->opcode << "]" << std::endl;
                }
                m_instret++;
                m_cycle++;
                if (!insn->is_last)
                {
                    m_insn_exec_event.schedule(1);
                }
                else
                {
                    if (info_logger_)
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
