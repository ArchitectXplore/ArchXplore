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

            abstractSystem() : RootTreeNode("System"), m_cpu_num(0)
            {
                g_system_ptr = this;
            };

            ~abstractSystem(){};

            virtual auto _build() -> void = 0;

            virtual auto _run(sparta::Scheduler::Tick tick) -> void = 0;

            virtual auto _createISS() -> iss::abstractISS::UniquePtr = 0;

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

            auto registerISS() -> void
            {
                for (auto cpuInfo : m_cpu_infos)
                {
                    cpuInfo.second.cpu->setISS(_createISS());
                }
            };

            auto getCPUPtr(const hartId_t &tid) -> cpu::abstractCPU *
            {
                if (m_cpu_infos.find(tid) != m_cpu_infos.end())
                {
                    return m_cpu_infos[tid].cpu;
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

            auto getCpuNumber() -> hartId_t {
                return m_cpu_num;
            };

            virtual auto addCPU(cpu::abstractCPU *cpu, const hartId_t &tid, const sparta::Clock::Frequency &freq) -> void
            {
                sparta_assert((cpu != nullptr));
                m_cpu_infos[tid] = {freq, cpu};
                m_cpu_num++;
            };

        public:
            std::mutex m_system_lock;

        protected:
            hartId_t m_cpu_num;
            processorInfoMap_t m_cpu_infos;
        };

    } // namespace system
} // namespace archXplore
