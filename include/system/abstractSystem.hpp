#pragma once

#include <unordered_map>

#include "sparta/sparta.hpp"
#include "sparta/simulation/GlobalTreeNode.hpp"
#include "sparta/utils/SpartaAssert.hpp"

#include "cpu/abstractCPU.hpp"
#include "iss/abstractISS.hpp"

namespace archXplore
{

    namespace system
    {
        class abstractSystem;

        extern abstractSystem *g_system_ptr;

        class abstractSystem : public sparta::RootTreeNode
        {
        private:
            struct processorInfo_t
            {
                sparta::Clock::Frequency freq;
                cpu::abstractCPU *cpu;
            };
            using processorInfoMap_t = std::unordered_map<iss::hartId_t, processorInfo_t>;

        public:
            // Delete Copy function
            abstractSystem(const abstractSystem &that) = delete;
            abstractSystem &operator=(const abstractSystem &that) = delete;

            abstractSystem() : RootTreeNode("System")
            {
                g_system_ptr = this;
            };

            ~abstractSystem()
            {
                this->enterTeardown();
            };

            virtual inline auto _build() -> void = 0;

            virtual inline auto _run(sparta::Scheduler::Tick tick) -> void = 0;

            virtual inline auto _createISS() -> iss::abstractISS::UniquePtr = 0;

            auto build() -> void
            {
                _build();
                registerISS();
            };

            auto run(sparta::Scheduler::Tick tick) -> void
            {
                _run(tick);
            };

            auto registerISS() -> void
            {
                for(auto cpuInfo : m_cpuInfos) {
                    cpuInfo.second.cpu->setISS(_createISS());
                }
            };

            virtual auto addCPU(cpu::abstractCPU *cpu, const iss::hartId_t &tid, const sparta::Clock::Frequency &freq) -> void
            {
                sparta_assert((cpu != nullptr));
                if (m_cpuInfos.find(tid) == m_cpuInfos.end())
                {
                    m_cpuInfos[tid] = {freq, cpu};
                }
                else
                {
                    sparta_throw("CPUs must have different hart id!");
                }
            };

        protected:
            processorInfoMap_t m_cpuInfos;
        };

    } // namespace system
} // namespace archXplore
