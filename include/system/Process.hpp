#pragma once

#include "Types.hpp"

namespace archXplore
{

    namespace system
    {

        struct Process
        {
            // Process ID
            ProcessID_t pid;
            // Process name
            std::string name;
            // Process executable path
            std::string executable;
            // Process command line arguments
            std::string arguments;
            // Process bootstrap hart
            ProcessID_t boot_hart;
            // Maximum number of hardware threads
            ProcessID_t max_harts;
        };

    } // namespace system

} // namespace archXplore
