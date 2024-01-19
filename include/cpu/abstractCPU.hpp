#pragma once

#include <atomic>

#include "sparta/simulation/Unit.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/simulation/Clock.hpp"

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

        enum cpuState_t
        {
            Power_off,
            Power_on
        };

        class abstractCPU : public sparta::Unit
        {
        public:
            // Delete copy-construct function
            abstractCPU(const abstractCPU &that) = delete;
            abstractCPU &operator=(const abstractCPU &that) = delete;

            abstractCPU(sparta::TreeNode *tn, const iss::hartId_t &tid, const sparta::Clock::Frequency &freq);

            ~abstractCPU();

            virtual auto reset() -> void = 0;

            virtual auto cleanUp() -> void {};

            virtual auto isRunning() -> bool
            {
                return m_state;
            };

            virtual auto powerOn() -> void
            {
                m_state = cpuState_t::Power_on;
                reset();
            };

            virtual auto powerOff() -> void
            {
                m_state = cpuState_t::Power_off;
            };

            virtual auto getISSPtr() -> iss::abstractISS *
            {
                return m_iss.get();
            };

            virtual auto getSystemPtr() -> system::abstractSystem *;

            auto getThreadID() -> const iss::hartId_t
            {
                return m_tid;
            };

            auto setISS(iss::abstractISS::UniquePtr iss) -> void
            {
                sparta_assert((iss != nullptr), "Setting iss which is nullptr");
                m_iss = std::move(iss);
                m_iss->init(this);
            };

        public:
            // CPU State
            cpuState_t m_state = cpuState_t::Power_off;
            // Unique Thread Id
            const iss::hartId_t m_tid;
            // Processor frequency
            const sparta::Clock::Frequency m_freq;

        private:
            // ISS Ptr
            iss::abstractISS::UniquePtr m_iss;
        };

    } // namespace iss
} // namespace archXplore
