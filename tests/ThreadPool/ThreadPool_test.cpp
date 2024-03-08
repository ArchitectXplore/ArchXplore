#include <iostream>
#include <vector>
#include <future>
#include "utils/ThreadPool.hpp"

using namespace archXplore::utils;

// Example usage
int main() {
    ThreadPool pool(4);

    // Enqueue tasks
    std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([i] { return i * i; }));
    }

    // Wait for tasks to complete
    for (auto &result : results) {
        std::cout << "Result: " << result.get() << std::endl;
    }

    return 0;
}