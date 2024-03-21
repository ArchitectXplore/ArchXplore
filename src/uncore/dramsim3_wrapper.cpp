#include "uncore/memory/dramsim3_wrapper.hpp"
namespace archXplore{
namespace uncore{
DRAMSim3Wrapper::DRAMSim3Wrapper(const std::string& config_file,
                                 const std::string& working_dir,
                                 std::function<void(uint64_t)> read_cb,
                                 std::function<void(uint64_t)> write_cb) :
    m_clock_period(0.0), m_queue_size(0), m_burst_size(0)
{
    // there is no way of getting DRAMsim3 to tell us what frequency
    // it is assuming, so we have to extract it ourselves
    m_memory_system.reset(new dramsim3::MemorySystem(config_file, working_dir,
                                       read_cb, write_cb));
    m_clock_period = m_memory_system->GetTCK();

    sparta_assert(m_clock_period, "DRAMsim3 wrapper failed to get clock\n");

    // we also need to know what transaction queue size DRAMsim3 is
    // using so we can stall when responses are blocked
    m_queue_size = m_memory_system->GetQueueSize();

    sparta_assert(m_queue_size, "DRAMsim3 wrapper failed to get queue size\n");


    // finally, get the data bus bits and burst length so we can add a
    // sanity check for the burst size
    unsigned int dataBusBits = m_memory_system->GetBusBits();
    unsigned int burstLength = m_memory_system->GetBurstLength();

    sparta_assert(dataBusBits && burstLength, "DRAMsim3 wrapper failed to get burst size\n");

     m_burst_size = dataBusBits * burstLength / 8;
}

void
DRAMSim3Wrapper::printStats()
{
    m_memory_system->PrintStats();
}

void
DRAMSim3Wrapper::resetStats()
{
    m_memory_system->ResetStats();
}

void
DRAMSim3Wrapper::setCallbacks(std::function<void(uint64_t)> read_complete,
                              std::function<void(uint64_t)> write_complete)
{
    m_memory_system->RegisterCallbacks(read_complete, write_complete);
}

bool
DRAMSim3Wrapper::canAccept(uint64_t addr, bool is_write) const
{
    return m_memory_system->WillAcceptTransaction(addr, is_write);
}

void
DRAMSim3Wrapper::enqueue(uint64_t addr, bool is_write)
{
    bool success = m_memory_system->AddTransaction(addr, is_write); // ? bool success M5_VAR_USED = 
    assert(success);
}

double
DRAMSim3Wrapper::clockPeriod() const
{
    return m_clock_period;
}

unsigned int
DRAMSim3Wrapper::queueSize() const
{
    return m_queue_size;
}

unsigned int
DRAMSim3Wrapper::burstSize() const
{
    return m_burst_size;
}

void
DRAMSim3Wrapper::tick()
{
    m_memory_system->ClockTick();
}


} // namespace uncore
} // namespace archXplore