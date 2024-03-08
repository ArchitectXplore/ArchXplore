#pragma once

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iox/signal_watcher.hpp"

#include "cpu/ThreadEvent.hpp"

namespace archXplore
{
    namespace iss
    {

        class EventPublisher
        {
            typedef iox::cxx::vector<cpu::ThreadEvent_t, 16384> Message_t;

        public:
            EventPublisher(const EventPublisher &rhs) = delete;
            EventPublisher &operator=(const EventPublisher &rhs) = delete;

            /**
             * @brief Constructor
             * @param app_name Application name
             * @param pid Process ID
             * @param tid Thread ID
             */
            EventPublisher(const std::string &app_name, const HartID_t &hart_id)
                : m_app_name(app_name), m_hart_id(hart_id)
            {
                // Configure publisher options
                iox::popo::PublisherOptions publisherOptions;
                publisherOptions.historyCapacity = 0;
                publisherOptions.subscriberTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

                // Create publisher
                m_publisher.reset(new iox::popo::Publisher<Message_t>(
                    {iox::RuntimeName_t(iox::TruncateToCapacity, app_name.c_str()),
                     iox::RuntimeName_t(iox::TruncateToCapacity, std::to_string(hart_id).c_str()),
                     "ThreadEvent"},
                    publisherOptions));
            }

            /**
             * @brief Destructor
             */
            ~EventPublisher() = default;

            /**
             * @brief Publish event
             * @param force_publish Force publish even if the sample is not full
             * @param args Arguments to forward to the event
             *
             * @return void
             */
            template <typename... Args>
            inline auto publish(const bool &force_publish, Args &&...args) -> void
            {
                m_event_buffer.emplace_back(std::forward<Args>(args)...);
                if (m_event_buffer.size() == m_event_buffer.capacity() || force_publish)
                {
                    while (!m_publisher->publishCopyOf(m_event_buffer) || !iox::hasTerminationRequested())
                    {
                        continue;
                    }
                }
            };

            /**
             * @brief Wait for subscribers
             *
             * @return void
             */
            inline auto waitForSubscribers() const -> void
            {
                while (!m_publisher->hasSubscribers() || !iox::hasTerminationRequested())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

        private:
            // Application name
            const std::string m_app_name;
            // Hart ID
            const HartID_t m_hart_id;
            // Publisher
            std::unique_ptr<iox::popo::Publisher<Message_t>> m_publisher;
            // Event buffer
            archXplore::iss::EventPublisher::Message_t m_event_buffer;
        };

    } // namespace iss

} // namespace archXplore
