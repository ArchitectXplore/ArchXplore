#include <iss/insnTunnel.hpp>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

archXplore::iss::insnTunnel<uint64_t> tunnel(1,1000000);

void producer_thread(){
    for(size_t i = 0 ; i < 1000000 ; i++){
        tunnel.push(i);
        // if(i % 1000 == 1)
    }
    tunnel.producer_exit();
    // std::cout << "PRODUCER EXIT!" << std::endl;
};

void consumer_thread(){
    for(size_t i = 0; i < 1000000 ; i++){
        uint64_t data;
        tunnel.pop(data);
        // if(i % 1000 == 1)
        assert(data == i);
    }
};

int main(int argc, char const *argv[])
{
    
    std::thread pt(producer_thread);
    std::thread ct(consumer_thread);

    pt.join();
    ct.join();

    return 0;
}