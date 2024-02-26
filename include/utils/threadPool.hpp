#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace archXplore
{

    namespace utils
    {
        class threadPool
        {
        public:
            threadPool(size_t numThreads) : stop(false)
            {
                for (size_t i = 0; i < numThreads; ++i)
                {
                    workers.emplace_back([this]
                                         {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !taskQueue.empty(); });

                        if (stop && taskQueue.empty()) {
                            return;
                        }

                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }

                    task(); // Execute the task
                } });
                }
            }

            ~threadPool()
            {
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    stop = true;
                }

                condition.notify_all();

                for (std::thread &worker : workers)
                {
                    worker.join();
                }
            }

            template <typename Func, typename... Args>
            auto enqueue(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
            {
                using ReturnType = decltype(func(args...));
                auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    if (stop)
                    {
                        throw std::runtime_error("enqueue on stopped threadPool");
                    }

                    taskQueue.emplace([task]()
                                      { (*task)(); });
                }

                condition.notify_one();
                return task->get_future();
            }

        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> taskQueue;
            std::mutex queueMutex;
            std::condition_variable condition;
            bool stop;
        };

    } // namespace utils

} // namespace archXplore
