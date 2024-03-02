#pragma once

// #include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>

namespace archXplore
{
    namespace utils
    {
        template <typename T>
        class threadSafeQueue
        {
        public:
            // Constructor
            threadSafeQueue() {};

            // Destructor
            ~threadSafeQueue() = default;

            // Set capacity of the queue
            auto setCapacity(size_t capacity) -> void
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.reserve(capacity);
            };

            // Enqueue a batch of elements
            auto pushBatch(std::vector<T> &values) -> void
            {
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    // Wait until the queue has enough elements
                    m_condition.wait(lock, [this]
                                    { return m_producing; });
                    m_queue.clear();
                    m_queue.swap(values);
                    m_producing = false;
                }
                m_condition.notify_one();
            };

            // Dequeue a batch of elements
            auto popBatch(std::vector<T> &values) -> void
            {
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    // Wait until the queue has enough elements
                    m_condition.wait(lock, [this]
                                    { return !m_producing; });
                    std::move(m_queue.rbegin(), m_queue.rend(), std::back_inserter(values));
                    m_producing = true;
                }
                m_condition.notify_one();
            };

            // Queue empty flag
            inline auto isEmpty() -> bool
            {
                return m_producing;
            };

        private:
            // Data maintainer & lock
            std::vector<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_condition;
            // Batch mode ping-pong condition
            bool m_producing = true;
        };
    } // namespace utils
} // namespace archXplore
