#pragma once

#include <queue>

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
                
                PARAMETER(uint32_t, fetch_width, 4, "Fetch width of the CPU in bytes");
            };

            class simpleCPU : public abstractCPU
            {
            public:
                simpleCPU(sparta::TreeNode *tn, const simpleCPUParams *params);

                ~simpleCPU();

                auto reset() -> void override;

                auto tick() -> void override;

                static const char name[];

            private:
                const simpleCPUParams *m_params;
                
                addr_t m_next_pc;

                std::queue<std::shared_ptr<instruction_t>> m_inst_buffer;

            };

        } // namespace simple
    }     // namespace cpu
} // namespace archXplore
