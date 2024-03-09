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
            typedef iox::cxx::vector<cpu::ThreadEvent_t, 1024> Message_t;

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

                auto app_name_str = iox::into<iox::lossy<iox::capro::IdString_t>>(app_name);
                auto instance_str = iox::into<iox::lossy<iox::capro::IdString_t>>(std::to_string(hart_id));
                auto event_name_str = iox::into<iox::lossy<iox::capro::IdString_t>>(std::string("ThreadEvent"));

                // Create subscriber
                m_subscriber.reset(new iox::popo::Subscriber<Message_t>(
                    {app_name_str, instance_str, event_name_str}, subscriberOptions));

                // Reset header to the beginning of the event buffer
                m_event_buffer_header = m_event_buffer.begin();

                init();
            }

            /**
             * @brief Destructor
             */
            ~EventSubscriber()
            {
                shutdown();
            };

            /**
             * @brief Initialize subscriber
             */
            auto init() -> void
            {
                while (m_subscriber->getSubscriptionState() != iox::SubscribeState::SUBSCRIBED)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            };

            /**
             * @brief Shutdown subscriber
             */
            auto shutdown() -> void
            {
                m_subscriber->unsubscribe();
            };

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
            iox::cxx::vector<cpu::ThreadEvent_t, 1024> m_event_buffer;
            // Header of the event buffer
            iox::cxx::vector<cpu::ThreadEvent_t, 1024>::iterator m_event_buffer_header;
        };

    } // namespace iss

} // namespace archXplore
