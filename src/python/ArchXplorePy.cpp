#include <cstdlib>
#include <iostream>
#include <csignal>

#include "python/EmbeddedModule.hpp"
#include "system/AbstractSystem.hpp"

namespace py = pybind11;

void systemCleanUp(int signum) noexcept
{
    auto system_ptr = archXplore::system::AbstractSystem::getSystemPtr();
    system_ptr->cleanUp();
    if (signum)
    {
        std::exit(signum);
    }
}

/*
 * This wrapper program runs python scripts using the python interpretter which
 * will be built into gem5. Its first argument is the script to run, and then
 * all subsequent arguments are passed to the python script as its argv.
 */

int main(int argc, const char **argv)
{
    // Add signal handling for SIGINT (Ctrl-C) so we can exit cleanly.
    signal(SIGINT, systemCleanUp);

    py::scoped_interpreter guard;

    // Embedded python doesn't set up sys.argv, so we'll do that ourselves.
    py::list py_argv;
    auto sys = py::module::import("sys");
    if (py::hasattr(sys, "argv"))
    {
        // sys.argv already exists, so grab that.
        py_argv = sys.attr("argv");
    }
    else
    {
        // sys.argv doesn't exist, so create it.
        sys.add_object("argv", py_argv);
    }
    // Clear out argv just in case it has something in it.
    py_argv.attr("clear")();

    if (argc < 2)
    {
        std::cerr << "Usage: main SCRIPT [arg] ..." << std::endl;
        std::exit(1);
    }

    // Fill it with our argvs.
    for (int i = 1; i < argc; i++)
        py_argv.append(argv[i]);

    // Actually call the script.
    py::eval_file(argv[1]);

    systemCleanUp(0);

    return 0;
}
