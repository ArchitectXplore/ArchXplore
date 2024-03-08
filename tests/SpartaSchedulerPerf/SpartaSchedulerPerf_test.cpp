#include "sparta/simulation/Unit.hpp"
#include "sparta/statistics/Counter.hpp"
#include "sparta/events/Event.hpp"
#include "sparta/events/StartupEvent.hpp"

#define PERF_CYCLE 100000000

class perfUnit : public sparta::Unit
{
public:
    perfUnit(sparta::TreeNode *parent)
        : Unit(parent),
          m_perf_event(&unit_event_set_, "PerfEvent",
                       CREATE_SPARTA_HANDLER(perfUnit, perfFunc)),
          m_perf_counter(&unit_stat_set_, "PerfCounter", "PerfCounter", sparta::Counter::CounterBehavior::COUNT_NORMAL)

    {
        sparta::StartupEvent(&unit_event_set_, CREATE_SPARTA_HANDLER(perfUnit, perfFunc));
    };

    ~perfUnit(){};

    void perfFunc()
    {
        m_perf_counter++;
        m_perf_event.schedule(1);
    };

private:
    sparta::Event<sparta::SchedulingPhase::Tick> m_perf_event;
    sparta::Counter m_perf_counter;
};

int main(int argc, char const *argv[])
{
    sparta::RootTreeNode rtn;

    sparta::Scheduler schduler("Schduler");

    sparta::Clock clk("Clock", &schduler);

    rtn.setClock(&clk);

    perfUnit perf_unit(&rtn);

    rtn.enterConfiguring();

    rtn.enterFinalized();

    schduler.finalize();

    // Start the clock
    auto start = std::chrono::high_resolution_clock::now();

    // Call the function to be profiled
    schduler.run(PERF_CYCLE, false, false);

    // Stop the clock
    auto stop = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    rtn.enterTeardown();

    double exec_time = ((double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den);

    double mips = PERF_CYCLE / exec_time / std::pow(10,6);

    // Output the profiling result
    std::cout << "Execution time: "
              << exec_time
              << " seconds" << std::endl;

    std::cout << "Million cycle per second: "
              << mips
              << " MIPS" << std::endl;

    return 0;
}
