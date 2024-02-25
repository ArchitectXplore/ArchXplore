
#include <memory>
#include <thread>
#include <iostream>
#include <chrono>
#include <deque>

#include "utils/threadSafeQueue.hpp"

// Number of elements to enqueue and dequeue
const int numElements = 100000000;

// Batch size for testing
const int batchSize = 16384;

archXplore::utils::threadSafeQueue<uint64_t> buffer;

void produceThreadFunc()
{
    for (size_t i = 0; i < numElements; i += batchSize)
    {
        std::deque<uint64_t> batchValues;
        for (int j = 0; j < batchSize && i + j < numElements; ++j)
        {
            batchValues.push_back(i + j);
        }
        buffer.pushBatch(batchValues);
    }
};

void consumerThreadFunc(){
    for (size_t i = 0; i < numElements; i += batchSize)
    {
        std::deque<uint64_t> batchValues;
        buffer.popBatch(batchValues);
    }
};

int main(int argc, char const *argv[])
{
    auto start = std::chrono::high_resolution_clock::now();

    std::thread pt(produceThreadFunc);
    std::thread ct(consumerThreadFunc);

    pt.join();
    ct.join();

    // Stop measuring time
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // Output the duration
    std::cout << "Elements size: " << numElements << std::endl;
    std::cout << "Batch size: " << batchSize << std::endl;
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Million operations per second: " << double(numElements / 1000000.0) / double(duration.count() / 1000.0) <<  std::endl;

    return 0;
}