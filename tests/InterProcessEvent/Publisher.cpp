
#include "iss/EventPublisher.hpp"

#include "iceoryx_posh/runtime/posh_runtime.hpp"

#define numElements 100000000
#define batchSize 16384

int main(int argc, char const *argv[])
{

    // Setup signal handler
    signal(SIGINT, [](int sig){
        std::cout << "Received SIGINT, stopping publisher" << std::endl;
        std::exit(sig);
    });

    // Initialize Posh runtime
    iox::runtime::PoshRuntime::initRuntime("iox-cpp-publisher");

    // Create publisher
    auto publisher = archXplore::iss::EventPublisher("TEST", 0);

    publisher.waitForSubscribers();

    std::cout << "Publisher ready to publish events" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numElements; i++)
    {
        auto event = archXplore::cpu::ThreadEvent_t(archXplore::cpu::ThreadEvent_t::SyscallApiTag, archXplore::cpu::SyscallAPI_t());
        event.event_id = i;
        if (i == numElements - 1)
        {
            event.is_last = true;
            publisher.publish(true, event);
        }
        else
        {
            publisher.publish(false, event);
        }
        // std::cout << "Published event " << i << std::endl;
    }

    // Stop measuring time
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // Output the duration
    std::cout << "Elements size: " << numElements << std::endl;
    std::cout << "Batch size: " << batchSize << std::endl;
    std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Million operations per second: " << double(numElements / 1000000.0) / double(duration.count() / 1000.0) << std::endl;

    return 0;
}