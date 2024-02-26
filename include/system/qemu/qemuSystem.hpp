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
                qemuSystem() 
                {
                    m_qemu_if = iss::qemu::qemuInterface::getInstance();
                };
                ~qemuSystem()
                {
                    m_qemu_if->qemu_shutdown();
                };

                virtual auto bootSystem() -> void override
                {
                    iss::qemu::m_simulated_cpu_number = getCPUCount();
                    boot();
                    // // Create Global Clock
                    // m_global_clock = m_clock_manager.makeRoot(this, "GlobalClock");
                    // setClock(m_global_clock.get());
                    // // Create Individual Clock for each CPU
                    // for (auto it : m_cpu_infos)
                    // {
                    //     const std::string clock_name = "CPU" + std::to_string(it.first) + "Clock";
                    //     const sparta::Clock::Frequency freq = it.second.freq;
                    //     sparta::Clock::Handle clock = m_clock_manager.makeClock(clock_name, m_global_clock, freq);
                    //     it.second.cpu->getContainer()->setClock(clock.get());
                    // };
                    // // Build ClockTree
                    // m_clock_manager.normalize();
                    // Enter Finalized, Build Resources and Start Connection
                    // enterConfiguring();
                    // enterFinalized();
                    // m_global_scheduler.finalize();
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

                auto _createISS() -> iss::abstractISS::UniquePtr override
                {
                    return std::make_unique<iss::qemu::qemuISS>();
                }

            private:
                // QEMU Interface Instance
                iss::qemu::qemuInterface::ptrType m_qemu_if;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
