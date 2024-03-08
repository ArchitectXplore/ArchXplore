#pragma once

#include <vector>
#include <deque>

#include "iss/AbstractISS.hpp"
#include "iss/EventSubscriber.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            class QemuISS : public AbstractISS
            {
            public:

                inline auto initCPUState() -> void override;

                inline auto generateFetchRequest(const Addr_t& addr, const size_t& fetch_size) -> void override;

                inline auto processFetchResponse(const uint8_t* data) -> std::vector<cpu::StaticInst_t> override;

                QemuISS();
            
                ~QemuISS();

            protected:

                inline auto handleThreadApi(const cpu::ThreadEvent_t& ev) -> void;

                inline auto handleSyscallApi(const cpu::ThreadEvent_t& ev) -> void;

                inline auto wakeUpMonitor() -> void override;

                inline auto initialize() -> void override;

            private:

                std::deque<std::vector<cpu::StaticInst_t>> m_fetch_buffer;

                std::unique_ptr<EventSubscriber> m_event_queue;

            };
        }
    } // namespace iss
} // namespace archXplore
