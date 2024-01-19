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
                : abstractCPU(
                      tn,
                      params->tid,
                      params->frequency),
                  m_insn_exec_event(&unit_event_set_, "InsnExecutionEvent",
                                    CREATE_SPARTA_HANDLER(simpleCPU, exec)){

                  };

            simpleCPU::~simpleCPU(){

            };

            auto simpleCPU::reset() -> void
            {
                exec();
            };

            auto simpleCPU::exec() -> void
            {
                if (isRunning())
                {
                    isa::traceInsnPtr_t insn;
                    auto iss = getISSPtr();
                    iss->receiveInstruction(insn);
                    info_logger_ << "Execute Instruction -> "
                                 << "uid[" << std::dec << insn->uid << "], "
                                 << "pc[" << std::hex << insn->pc << "], "
                                 << "opcode[" << std::hex << insn->opcode << "]" << std::endl;

                    m_insn_exec_event.schedule(1);
                }
            };

            REGISTER_SPARTA_UNIT(simpleCPU, simpleCPUParams);

        }
    }
}
