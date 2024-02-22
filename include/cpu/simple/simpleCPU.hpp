#pragma once

#include "sparta/sparta.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/events/Event.hpp"
#include "sparta/statistics/Counter.hpp"

#include "cpu/abstractCPU.hpp"

namespace archXplore
{
    namespace cpu
    {
        namespace simple
        {

            class simpleCPUParams : public sparta::ParameterSet
            {
            public:
                simpleCPUParams(sparta::TreeNode *parent)
                    : ParameterSet(parent){};

                PARAMETER(hartId_t, tid, 0, "Thread ID");
                PARAMETER(sparta::Clock::Frequency, frequency, 1000, "CPU frequency")
            };

            class simpleCPU : public abstractCPU
            {
            public:
                simpleCPU(sparta::TreeNode *tn, const simpleCPUParams *params);

                ~simpleCPU();

                auto reset() -> void override;

                auto exec() -> void;

                static const char *name;

            private:
                sparta::Event<sparta::SchedulingPhase::Tick> m_insn_exec_event;
                sparta::Counter m_cycle;
                sparta::Counter m_instret;
            };

        } // namespace simple
    }     // namespace cpu
} // namespace archXplore
