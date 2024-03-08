#pragma once

#include <unistd.h>

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"

#include "system/AbstractSystem.hpp"
#include "iss/qemu/QemuISS.hpp"

#include "utils/subprocess.h"

namespace archXplore
{
    namespace system
    {
        namespace qemu
        {

            class QemuSystem : public AbstractSystem
            {
            public:
                QemuSystem()
                {
                    // Initialize RouDi App
                    auto app_name = iox::RuntimeName_t(iox::TruncateToCapacity, getAppName().c_str());
                    iox::runtime::PoshRuntime::initRuntime(app_name);
                };

                ~QemuSystem(){};

                auto bootSystem() -> void override
                {

                };

                auto _createISS() -> std::unique_ptr<iss::AbstractISS> override
                {
                    return std::make_unique<iss::qemu::QemuISS>();
                }

            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
