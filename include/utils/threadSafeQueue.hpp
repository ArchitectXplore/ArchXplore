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
            threadSafeQueue() :  m_producing(true){};

            // Destructor
            ~threadSafeQueue() = default;

            // Set capacity of the queue
            auto setCapacity(size_t capacity) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_queue.reserve(capacity);
            };

            // Enqueue a batch of elements
            auto pushBatch(std::vector<T> &values) -> void
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
            auto tryPushBatch(std::vector<T> &values) -> bool
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


            // Dequeue a batch of elements
            auto popBatch(std::vector<T> &values) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                m_condition.wait(lock, [this]
                                 { return !m_producing; });
                std::copy(m_queue.rbegin(), m_queue.rend(), std::back_inserter(values));
                m_queue.clear();
                m_producing = true;
                m_condition.notify_one();
            };

            // Try dequeue a batch of elements
            auto tryPopBatch(std::vector<T> &values) -> bool
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                if (m_producing)
                    return false;

                std::copy(m_queue.rbegin(), m_queue.rend(), std::back_inserter(values));
                m_queue.clear();
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
            // std::deque<T> m_queue;
            std::vector<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_condition;
            // Batch mode ping-pong condition
            bool m_producing;
        };
    } // namespace utils
} // namespace archXplore
