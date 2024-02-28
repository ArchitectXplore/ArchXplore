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
                std::lock_guard<std::mutex> lock(m_mutex);
                if(isEmpty()) {
                    m_queue.swap(values);
                } else {
                    m_queue.insert(m_queue.end(), values.begin(), values.end());
                    values.clear();
                }
                m_condition.notify_one();
            };

            // Enqueue a batch of elements
            auto tryPushBatch(std::vector<T> &values) -> bool
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if(!isEmpty()) {
                    return false;
                } else {
                    m_queue.swap(values);
                    m_condition.notify_one();
                    return true;
                }
            };

            // Dequeue a batch of elements
            auto popBatch(std::vector<T> &values) -> void
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                // Wait until the queue has enough elements
                m_condition.wait(lock, [this]
                                 { return !isEmpty(); });
                std::cout << "Popping batch of size " << m_queue.size() << std::endl;
                std::move(m_queue.rbegin(), m_queue.rend(), std::back_inserter(values));
                m_queue.clear();
                m_condition.notify_one();
            };

            // Queue empty flag
            inline auto isEmpty() -> bool
            {
                return m_queue.empty();
            };

        private:
            // Data maintainer & lock
            // std::deque<T> m_queue;
            std::vector<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_condition;
        };
    } // namespace utils
} // namespace archXplore
