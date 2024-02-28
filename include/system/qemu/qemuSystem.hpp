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

                /**
                 * Boot the system.
                 */
                virtual auto bootSystem() -> void override
                {
                    iss::qemu::m_simulated_cpu_number = getCPUCount();
                    boot();
                };


                auto boot() -> void
                {
                    // Plugin library path
                    std::string plugin_path = executablePath() + "/libqemuInterface_plugin.so";

                    // Simulating command-line arguments
                    std::vector<std::string> args = {"QEMU", "-plugin", plugin_path, m_workload_path};

                    // Add workload arguments
                    if (m_workload_args.size() > 0)
                    {
                        args.insert(args.end(), m_workload_args.begin(), m_workload_args.end());
                    }

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
