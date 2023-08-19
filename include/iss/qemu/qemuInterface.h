#pragma once

#include <thread>
#include <mutex>

namespace archXplore {
namespace iss {
namespace qemu {

extern "C" {

class qemuInterface
{

public:
    // Delected function
    qemuInterface(qemuInterface&) = delete;
    qemuInterface& operator=(const qemuInterface&) = delete;

public:
    ~qemuInterface();

    static qemuInterface& getInstance();
    static std::thread& createQemuThread();
private:
    qemuInterface(/* args */);

private:
    std::mutex m_exit_lock;
};

qemuInterface::qemuInterface(/* args */)
{
}

qemuInterface::~qemuInterface()
{
}


}
    
} // namespace qemu
} // namespace iss
} // namespace archXplore
