#pragma once

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"

#include "system/abstractSystem.hpp"
#include "iss/qemu/qemuInterface.hpp"
#include "iss/qemu/qemuISS.hpp"

namespace archXplore
{
    namespace system
    {
        namespace qemu
        {

            class qemuSystem : public abstractSystem
            {
            public:
                qemuSystem() : m_global_scheduler("GlobalScheduler", getSearchScope()),
                               m_clock_manager(&m_global_scheduler)
                {
                    m_qemu_if = iss::qemu::qemuInterface::getInstance();
                };
                ~qemuSystem()
                {
                    m_qemu_if->qemu_shutdown();
                };

                auto _build() -> void override
                {
                    // Create Global Clock
                    m_global_clock = m_clock_manager.makeRoot(this, "GlobalClock");
                    setClock(m_global_clock.get());
                    // Create Individual Clock for each CPU
                    for (auto it : m_cpu_infos)
                    {
                        const std::string clock_name = "CPU" + std::to_string(it.first) + "Clock";
                        const sparta::Clock::Frequency freq = it.second.freq;
                        sparta::Clock::Handle clock = m_clock_manager.makeClock(clock_name, m_global_clock, freq);
                        it.second.cpu->getContainer()->setClock(clock.get());
                    };
                    // Build ClockTree
                    m_clock_manager.normalize();
                    // Enter Finalized, Build Resources and Start Connection
                    enterConfiguring();
                    enterFinalized();
                    m_global_scheduler.finalize();
                };

                auto boot() -> void
                {
                    // Plugin library path
                    std::string plugin_path = executablePath() + "/libqemuInterface_plugin.so";

                    // Simulating command-line arguments
                    std::vector<std::string> args = {"QEMU", "-plugin", plugin_path, m_workload_path};

                    // Create argc and argv
                    int argc = static_cast<int>(args.size());
                    char **argv = new char *[argc];

                    for (int i = 0; i < argc; ++i)
                    {
                        argv[i] = new char[args[i].size() + 1];
                        std::strcpy(argv[i], args[i].c_str());
                    }

                    // Your program logic goes here...
                    boot({argc, argv, nullptr});
                }

                auto boot(const iss::qemu::qemuArgs_t &args) -> void
                {
                    // Your program logic goes here...
                    m_qemu_if->bootQemuThread(args);
                };

                auto _run(sparta::Scheduler::Tick tick) -> void override
                {
                    boot();
                    m_global_scheduler.run(tick, false, false);
                };

                auto _createISS() -> iss::abstractISS::UniquePtr override
                {
                    iss::qemu::m_simulated_cpu_number++;
                    return std::make_unique<iss::qemu::qemuISS>();
                }

            private:
                // Sparta Clock and Scheduler
                sparta::Scheduler m_global_scheduler;
                sparta::ClockManager m_clock_manager;
                sparta::Clock::Handle m_global_clock;
                // QEMU Interface Instance

                iss::qemu::qemuInterface::ptrType m_qemu_if;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
