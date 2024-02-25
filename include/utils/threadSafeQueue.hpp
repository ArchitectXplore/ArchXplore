#pragma once

#include <deque>
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
            threadSafeQueue() :  m_producing(true){};

            // Destructor
            ~threadSafeQueue() = default;

            // Enqueue an element
            auto push(const T &value) -> void
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.emplace_back(value);
                m_condition.notify_one();
            };

            // Enqueue a batch of elements
            auto pushBatch(std::deque<T> &values) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                m_condition.wait(lock, [this]
                                 { return m_producing; });
                m_queue.swap(values);
                m_producing = false;
                m_condition.notify_one();
            };

            // Try enqueue a batch of elements
            auto tryPushBatch(std::deque<T> &values) -> bool
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                if (!m_producing)
                    return false;

                m_condition.wait(lock, [this]
                                 { return m_producing; });
                m_queue.swap(values);
                m_producing = false;
                m_condition.notify_one();
                return true;
            };

            // Dequeue an element
            auto pop(T &value) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue is not empty
                m_condition.wait(lock, [this]
                                 { return !m_queue.empty(); });
                value = m_queue.front();
                m_queue.pop_front();
                return value;
            };

            // Dequeue a batch of elements
            auto popBatch(std::deque<T> &values) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                m_condition.wait(lock, [this]
                                 { return !m_producing; });
                values.swap(m_queue);
                m_producing = true;
                m_condition.notify_one();
            };

            // Try dequeue a batch of elements
            auto tryPopBatch(std::deque<T> &values) -> bool
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                if (m_producing)
                    return false;

                values.swap(m_queue);
                m_producing = true;
                m_condition.notify_one();

                return true;
            };

            // Queue empty flag
            auto isEmpty() -> bool
            {
                return m_queue.empty();
            };

        private:
            // Data maintainer & lock
            std::deque<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_condition;
            // Batch mode ping-pong condition
            bool m_producing;
        };
    } // namespace utils
} // namespace archXplore
