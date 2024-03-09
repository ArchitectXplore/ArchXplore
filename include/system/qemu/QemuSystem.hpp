#pragma once

#include <unistd.h>

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"

#include "system/AbstractSystem.hpp"
#include "iss/qemu/QemuISS.hpp"

#include "utils/Subprocess.h"

namespace archXplore
{
    namespace system
    {
        namespace qemu
        {

            class QemuSystem : public AbstractSystem
            {
            public:
                /**
                 * @brief Construct a new QemuSystem object
                 */
                QemuSystem()
                {
                    int result;

                    std::system("pkill iox-roudi");

                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    // Boot RouDi Process
                    std::string roudi_exe = executablePath() + "/iox-roudi";

                    const char *roudi_cmd[] = {roudi_exe.c_str(), NULL};

                    result = subprocess_create(roudi_cmd, 0, &m_roudi_subprocess);

                    if (result != 0)
                    {
                        throw std::runtime_error("Failed to create RouDi subprocess");
                    }

                    // Initialize RouDi App
                    auto app_name = iox::RuntimeName_t(iox::TruncateToCapacity, getAppName().c_str());
                    iox::runtime::PoshRuntime::initRuntime(app_name);
                };
                /**
                 * @brief Destroy the QemuSystem object
                 */
                ~QemuSystem()
                {
                    cleanUp();
                };

                /**
                 * @brief Clean up the system and release resources.
                 */
                auto cleanUp() noexcept(false) -> void override
                {
                    // Shutdown QEMU Subprocesses
                    for (auto &qemu_subprocess : m_qemu_subprocesses)
                    {
                        subprocess_destroy(&qemu_subprocess);
                    }
                };

                /**
                 * @brief Boot the system.
                 */
                auto bootSystem() -> void override
                {
                    HartID_t hart_used = 0;
                    for (auto &process : m_processes)
                    {
                        process->boot_hart = hart_used;
                        hart_used = hart_used + process->max_harts;
                        sparta_assert(hart_used <= getCPUCount(), "Too many harts requested");
                        newQemuProcess(process);
                    }
                };

                /**
                 * @brief New Qemu Process
                 * @param guest_process Process to be booted
                 */
                auto newQemuProcess(Process *guest_process) -> void
                {
                    // Boot QEMU Process

                    subprocess_s qemu_subprocess;

                    std::vector<const char *> command_vec;
                    // QEMU Location
                    std::string qemu_location = executablePath() + "/qemu/qemu-riscv64";
                    command_vec.push_back(qemu_location.c_str());
                    // QEMU Plugin
                    std::string plugin_prefix = "-plugin";
                    command_vec.push_back(plugin_prefix.c_str());

                    std::string plugin_cmd = executablePath() + "/libInstrumentPlugin.so" + ",AppName=" + getAppName() +
                                             ",ProcessID=" + std::to_string(guest_process->pid) +
                                             ",BootHart=" + std::to_string(guest_process->boot_hart) +
                                             ",MaxHarts=" + std::to_string(guest_process->max_harts);
                    command_vec.push_back(plugin_cmd.c_str());
                    // QEMU guest executable
                    std::string executable_path = guest_process->executable;
                    command_vec.push_back(executable_path.c_str());
                    // QEMU guest arguments
                    for (auto &arg : guest_process->arguments)
                    {
                        command_vec.push_back(arg.c_str());
                    }
                    // QEMU Command Termination
                    command_vec.push_back(NULL);

                    if (SPARTA_EXPECT_FALSE(m_debug_logger))
                    {
                        std::string log;
                        log += "Starting QEMU for process ";
                        log += std::to_string(guest_process->pid);
                        log += " with command: ";
                        for (auto &cmd : command_vec)
                        {
                            if (cmd != NULL)
                            {
                                log += std::string(cmd) + " ";
                            }
                        }
                        m_debug_logger << log << std::endl;
                    }

                    const char *const *qemu_cmd = command_vec.data();

                    if (!subprocess_create(qemu_cmd, 0, &qemu_subprocess))
                    {
                        m_qemu_subprocesses.push_back(qemu_subprocess);
                    }
                    else
                    {
                        throw std::runtime_error("Failed to create Qemu subprocess");
                    }

                    // Boot harts for this process
                    getCPUPtr(guest_process->boot_hart)->scheduleStartupEvent();
                    for (HartID_t hart_offset = 1; hart_offset < guest_process->max_harts; hart_offset++)
                    {
                        getCPUPtr(guest_process->boot_hart + hart_offset)->scheduleWakeUpMonitorEvent();
                    }
                };

                /**
                 * @brief Create an instance of the ISS.
                 * @return A unique pointer to the ISS.
                 */
                auto createISS() -> std::unique_ptr<iss::AbstractISS> override
                {
                    return std::make_unique<iss::qemu::QemuISS>();
                }

            private:
                // QEMU Subprocesses
                std::vector<subprocess_s> m_qemu_subprocesses;
                // RouDi Subprocess
                subprocess_s m_roudi_subprocess;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
