#pragma once

#include <queue>

#include "cpu/AbstractCPU.hpp"

namespace archXplore
{
    namespace cpu
    {
        namespace simple
        {

            class SimpleCPUParams : public sparta::ParameterSet
            {
            public:
                SimpleCPUParams(sparta::TreeNode *parent)
                    : ParameterSet(parent){};
                
                PARAMETER(uint32_t, fetch_width, 4, "Fetch width of the CPU in bytes");
            };

            class SimpleCPU : public AbstractCPU
            {
            public:
                SimpleCPU(sparta::TreeNode *tn, const SimpleCPUParams *params);

                ~SimpleCPU();

                auto reset() -> void override;

                auto tick() -> void override;

                static const char name[];

            private:
                const SimpleCPUParams *m_params;
                
                Addr_t m_next_pc;

                std::queue<std::shared_ptr<StaticInst_t>> m_inst_buffer;

            };

        } // namespace simple
    }     // namespace cpu
} // namespace archXplore
