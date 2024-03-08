
#include "iss/EventSubscriber.hpp"

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"


#define numElements 100000000
#define batchSize 16384

int main(int argc, char const *argv[])
{

    // Initialize Posh runtime
    iox::runtime::PoshRuntime::initRuntime("iox-cpp-subscriber");

    // Create publisher
    auto subscriber = archXplore::iss::EventSubscriber("TEST", 0);

    auto start = std::chrono::high_resolution_clock::now();

    while (!iox::hasTerminationRequested())
    {
        auto &event = subscriber.front();
        if (event.is_last)
        {
            subscriber.popFront();
            break;
        }
        else
        {
            subscriber.popFront();
        }
        // std::cout << "Received event: " << event.event_id << std::endl;
    }

    return 0;
}