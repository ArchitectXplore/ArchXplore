#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace archXplore
{
    namespace utils
    {
        template <typename T>
        class threadSafeQueue
        {
        public:
            // Constructor
            threadSafeQueue() : m_producer_exited(false){};

            // Destructor
            ~threadSafeQueue() = default;

            // Enqueue an element
            auto push(const T &value) -> void
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(value);
                m_condition.notify_one();
            };

            // Enqueue a batch of elements
            auto pushBatch(const std::deque<T> &values) -> void
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (const T &value : values)
                {
                    m_queue.push(value);
                }
                m_condition.notify_one();
            };

            // Dequeue an element
            auto pop(T &value) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue is not empty
                m_condition.wait(lock, [this]
                                 { return !m_queue.empty(); });
                value = m_queue.front();
                m_queue.pop();
                return value;
            };

            // Dequeue a batch of elements
            auto popBatch(std::deque<T> &values, std::size_t batch_size) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                m_condition.wait(lock, [this, batch_size]
                                 { return m_producer_exited || (m_queue.size() >= batch_size); });

                for (std::size_t i = 0; (i < batch_size) && !m_queue.empty(); ++i)
                {
                    values.push_back(m_queue.front());
                    m_queue.pop();
                }
            };

            // Producer doesn't produce data anymore
            auto producerExit() -> void
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_producer_exited = true;
            };

            // Queue empty flag
            auto isEmpty() -> bool
            {
                return m_queue.empty();
            };

        private:
            // Producer Status
            bool m_producer_exited;
            // Data maintainer & lock
            std::queue<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_condition;
        };
    } // namespace utils
} // namespace archXplore
