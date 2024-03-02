#pragma once

#include <vector>
#include <deque>

#include "iss/abstractISS.hpp"
#include "iss/qemu/qemuInterface.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            class qemuISS : public abstractISS
            {
            public:

                inline auto initCPUState() -> void override;

                inline auto generateFetchRequest(const addr_t& addr, const size_t& fetch_size) -> void override;

                inline auto processFetchResponse(const uint8_t* data) -> std::vector<cpu::instruction_t> override;

                qemuISS();
            
                ~qemuISS();

            protected:

                inline auto handleThreadApi(const cpu::threadEvent_t& ev) -> void;

                inline auto handleSyscallApi(const cpu::threadEvent_t& ev) -> void;

                inline auto wakeUpMonitor() -> void override;

                inline auto initialize() -> void override;

            private:

                std::deque<std::vector<cpu::instruction_t>> m_fetch_buffer;

                hartEventQueue *m_event_queue;
            };
        }
    } // namespace iss
} // namespace archXplore
