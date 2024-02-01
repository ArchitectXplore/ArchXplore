#pragma once

#include <unordered_map>
#include <mutex>

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
            using processorInfoMap_t = std::unordered_map<hartId_t, processorInfo_t>;

        public:
            // Delete Copy function
            abstractSystem(const abstractSystem &that) = delete;
            abstractSystem &operator=(const abstractSystem &that) = delete;

            abstractSystem() : RootTreeNode("System")
            {
                g_system_ptr = this;
            };

            ~abstractSystem(){};

            virtual auto _build() -> void = 0;

            virtual auto _run(sparta::Scheduler::Tick tick) -> void = 0;

            virtual auto _createISS() -> iss::abstractISS::UniquePtr = 0;

            virtual auto _cleanUp() -> void{};

            auto build() -> void
            {
                _build();
                registerISS();
                enterTeardown();
            };

            auto run(sparta::Scheduler::Tick tick) -> void
            {
                _run(tick);
            };

            auto cleanUp() -> void
            {
                for (auto cpuInfo : m_cpuInfos)
                {
                    cpuInfo.second.cpu->cleanUp();
                }
                _cleanUp();
            };

            auto registerISS() -> void
            {
                for (auto cpuInfo : m_cpuInfos)
                {
                    cpuInfo.second.cpu->setISS(_createISS());
                }
            };

            auto getCPUPtr(const hartId_t &tid) -> cpu::abstractCPU *
            {
                if (m_cpuInfos.find(tid) != m_cpuInfos.end())
                {
                    return m_cpuInfos[tid].cpu;
                }
                else
                {
                    sparta_throw("CPUs must have different hart id!");
                    return nullptr;
                }
            };

            static auto getSystemPtr() -> abstractSystem* {
                sparta_assert((g_system_ptr != nullptr), "Can't get system pointer before build it\n");
                return g_system_ptr;
            };

            virtual auto addCPU(cpu::abstractCPU *cpu, const hartId_t &tid, const sparta::Clock::Frequency &freq) -> void
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

        public:
            std::mutex m_system_lock;

        protected:
            processorInfoMap_t m_cpuInfos;
        };

    } // namespace system
} // namespace archXplore
