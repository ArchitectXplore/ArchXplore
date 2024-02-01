#pragma once

#include <atomic>

#include "sparta/simulation/Unit.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/simulation/Clock.hpp"
#include "sparta/statistics/Counter.hpp"

#include "iss/abstractISS.hpp"

// Forward Declaration
namespace archXplore::system
{
    class abstractSystem;
};

namespace archXplore
{

    namespace cpu
    {

        enum cpuStatus_t
        {
            INACTIVE,
            ACTIVE,
            BLOCKED_COMM,
            BLOCKED_MUTEX,
            BLOCKED_BARRIER,
            BLOCKED_COND,
            BLOCKED_JOIN,
            COMPLETED,
            NUM_STATUSES
        };

        class abstractCPU : public sparta::Unit
        {
        public:
            // Delete copy-construct function
            abstractCPU(const abstractCPU &that) = delete;
            abstractCPU &operator=(const abstractCPU &that) = delete;

            abstractCPU(sparta::TreeNode *tn, const hartId_t &tid, const sparta::Clock::Frequency &freq);

            ~abstractCPU();

            virtual auto reset() -> void = 0;

            virtual auto cleanUp() -> void{};

            virtual auto tick() -> void {
                m_cycle++;
            };

            auto isRunning() const -> bool
            {
                return m_status > cpuStatus_t::INACTIVE && m_status < cpuStatus_t::COMPLETED;
            };
            auto isBlocked() const -> bool
            {
                return m_status > cpuStatus_t::ACTIVE && m_status < cpuStatus_t::COMPLETED;
            };
            auto isCompleted() const -> bool
            {
                return m_status == cpuStatus_t::COMPLETED;
            };
            auto getISSPtr() -> iss::abstractISS *
            {
                return m_iss.get();
            };
            auto getSystemPtr() -> system::abstractSystem *;
            auto getThreadID() -> const hartId_t
            {
                return m_tid;
            };
            auto setISS(iss::abstractISS::UniquePtr iss) -> void
            {
                sparta_assert((iss != nullptr), "Setting iss to nullptr");
                m_iss = std::move(iss);
                m_iss->setCPU(this);
            };

        public:
            // CPU Status
            cpuStatus_t m_status;
            // Cycle counter
            sparta::Counter m_cycle;
            // Instruction retired counter
            sparta::Counter m_instret;
            // Unique Thread Id
            const hartId_t m_tid;
            // Processor frequency
            const sparta::Clock::Frequency m_freq;

        private:
            // ISS Ptr
            iss::abstractISS::UniquePtr m_iss;
        };

    } // namespace iss
} // namespace archXplore
