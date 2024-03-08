#pragma once

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"

#include "cpu/ThreadEvent.hpp"

namespace archXplore
{
    namespace iss
    {

        class EventSubscriber
        {
            typedef iox::cxx::vector<cpu::ThreadEvent_t, 16384> Message_t;

        public:
            EventSubscriber(const EventSubscriber &rhs) = delete;
            EventSubscriber &operator=(const EventSubscriber &rhs) = delete;

            /**
             * @brief Constructor
             * @param app_name Application name
             * @param pid Process ID
             * @param tid Thread ID
             */
            EventSubscriber(const std::string &app_name, const HartID_t &hart_id)
                : m_app_name(app_name), m_hart_id(hart_id)
            {
                // Configure subscriber options
                iox::popo::SubscriberOptions subscriberOptions;
                subscriberOptions.queueCapacity = 4;
                subscriberOptions.historyRequest = 0;
                subscriberOptions.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;

                // // Create subscriber
                m_subscriber.reset(new iox::popo::Subscriber<Message_t>(
                    {iox::RuntimeName_t(iox::TruncateToCapacity, app_name.c_str()),
                     iox::RuntimeName_t(iox::TruncateToCapacity, std::to_string(hart_id).c_str()),
                     "ThreadEvent"},
                    subscriberOptions));
            }

            /**
             * @brief Destructor
             */
            ~EventSubscriber() = default;

            /**
             * @brief Get front of event buffer
             * @return Front of event buffer
             */
            inline auto front() -> cpu::ThreadEvent_t &
            {
                if (m_event_buffer_header == m_event_buffer.end())
                {
                    take();
                }
                return *m_event_buffer_header;
            };

            /**
             * @brief Pop front of event buffer
             */
            inline auto popFront() -> void
            {
                m_event_buffer_header++;
            };

            /**
             * @brief Take event buffer from subscriber
             */
            inline auto take() -> void
            {
                bool take_successful = false;
                while (!take_successful)
                {
                    auto maybeEvent = m_subscriber->take();
                    if (maybeEvent.has_value())
                    {
                        m_event_buffer = std::move(*maybeEvent.value());
                        m_event_buffer_header = m_event_buffer.begin();
                        take_successful = true;
                    }
                }
            };

            /**
             * @brief Try to take event buffer from subscriber
             * @return True if successful, false otherwise
             */
            inline auto tryTake() -> bool
            {
                auto maybeEvent = m_subscriber->take();
                if (maybeEvent.has_value())
                {
                    m_event_buffer = std::move(*maybeEvent.value());
                    m_event_buffer_header = m_event_buffer.begin();
                    return true;
                }
                else
                {
                    return false;
                }
            };

        private:
            // Application name
            const std::string m_app_name;
            // Hart ID
            const HartID_t m_hart_id;
            // Subscriber
            std::unique_ptr<iox::popo::Subscriber<Message_t>> m_subscriber;
            // Event buffer
            iox::cxx::vector<cpu::ThreadEvent_t, 16384> m_event_buffer;
            // Header of the event buffer
            iox::cxx::vector<cpu::ThreadEvent_t, 16384>::iterator m_event_buffer_header;
        };

    } // namespace iss

} // namespace archXplore
