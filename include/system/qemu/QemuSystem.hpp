#pragma once

#include <unistd.h>
#include <iostream>

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"

#include "system/AbstractSystem.hpp"
#include "iss/qemu/QemuISS.hpp"

#include "utils/Subprocess.hpp"

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
                    // Initialize RouDi App
                    auto app_name = iox::RuntimeName_t(iox::TruncateToCapacity, getAppName().c_str());
                    iox::runtime::PoshRuntime::initRuntime(app_name);
                };
                /**
                 * @brief Destroy the QemuSystem object
                 */
                ~QemuSystem(){};

                /**
                 * @brief Clean up the system and release resources.
                 */
                auto cleanUp() noexcept(false) -> void override
                {
                    // Shutdown QEMU Subprocesses
                    for (auto &qemu_subprocess : m_qemu_subprocesses)
                    {
                        qemu_subprocess->kill(2);
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

                    std::vector<std::string> command_vec;
                    // QEMU Location
                    std::string qemu_location = executablePath() + "/qemu/qemu-riscv64";
                    command_vec.push_back(qemu_location);
                    // QEMU Plugin
                    std::string plugin_prefix = "-plugin";
                    command_vec.push_back(plugin_prefix);

                    std::string plugin_cmd = executablePath() + "/libInstrumentPlugin.so" + ",AppName=" + getAppName() +
                                             ",ProcessID=" + std::to_string(guest_process->pid) +
                                             ",BootHart=" + std::to_string(guest_process->boot_hart) +
                                             ",MaxHarts=" + std::to_string(guest_process->max_harts);
                    command_vec.push_back(plugin_cmd);
                    // QEMU guest executable
                    std::string executable_path = guest_process->executable;
                    command_vec.push_back(executable_path);
                    // QEMU guest arguments
                    for (auto &arg : guest_process->arguments)
                    {
                        command_vec.push_back(arg.c_str());
                    }

                    if (SPARTA_EXPECT_FALSE(m_debug_logger))
                    {
                        std::string log;
                        log += "Starting QEMU for process ";
                        log += std::to_string(guest_process->pid);
                        log += " with command: ";
                        for (auto &cmd : command_vec)
                        {
                            log += std::string(cmd) + " ";
                        }
                        m_debug_logger << log << std::endl;
                    }

                    m_qemu_subprocesses.emplace_back(new subprocess::Popen(
                        command_vec,
                        subprocess::input{subprocess::PIPE},
                        subprocess::output{subprocess::PIPE},
                        subprocess::error{subprocess::PIPE}));
                    
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
                std::vector<std::unique_ptr<subprocess::Popen>> m_qemu_subprocesses;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
