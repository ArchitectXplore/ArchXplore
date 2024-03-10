#include "python/EmbeddedModule.hpp"

#include "iss/AbstractISS.hpp"
#include "iss/qemu/QemuISS.hpp"

#include "system/AbstractSystem.hpp"
#include "system/qemu/QemuSystem.hpp"
#include "system/Process.hpp"

#include "ClockedObject.hpp"

namespace archXplore
{
    namespace python
    {

        void bindCommonModules(pybind11::module_ &parent)
        {
            // Bind Process
            pybind11::class_<archXplore::system::Process>(parent, "Process")
                .def(pybind11::init<>())
                .def_readonly("pid", &archXplore::system::Process::pid, "Process ID")
                .def_readwrite("name", &archXplore::system::Process::name, "Process name")
                .def_readwrite("executable", &archXplore::system::Process::executable, "Process executable path")
                .def_readwrite("arguments", &archXplore::system::Process::arguments, "Process arguments")
                .def_readonly("boot_hart", &archXplore::system::Process::boot_hart, "Boot hart ID")
                .def_readwrite("max_harts", &archXplore::system::Process::max_harts, "Maximum number of harts");

            // Bind ClockedObject
            pybind11::class_<ClockedObject, PyClockedObject, sparta::TreeNode>(parent, "ClockedObject")
                .def(pybind11::init<sparta::TreeNode *, const std::string &>())
                .def("setClockDomain", &ClockedObject::setClockDomain,
                     pybind11::return_value_policy::reference, "Set the clock domain of the object")
                .def("setClockFrequency", &ClockedObject::setClockFrequency,
                     pybind11::return_value_policy::reference, "Set the clock frequency of the object")
                .def("setRank", &ClockedObject::setRank,
                     pybind11::return_value_policy::reference, "Set the rank of the object")
                .def("buildTopology", &ClockedObject::buildTopology, "Build object's topology");

            // Bind ISS Modules
            auto iss = python::EmbeddedModule::createSubPackage(parent, "iss");
            pybind11::class_<archXplore::iss::AbstractISS>(iss, "__AbstractISS");
            pybind11::class_<archXplore::iss::qemu::QemuISS, archXplore::iss::AbstractISS>(iss, "QemuISS")
                .def(pybind11::init<>());

            /* Bind System Modules */
            auto system = python::EmbeddedModule::createSubPackage(parent, "system");
            // Bind AbstractSystem
            pybind11::class_<archXplore::system::AbstractSystem, sparta::RootTreeNode>(system, "__AbstractSystem", pybind11::dynamic_attr())
                .def("run", &archXplore::system::AbstractSystem::run,
                     pybind11::arg("tick") = sparta::Scheduler::INDEFINITE,
                     "Run the system for a given number of ticks (default: indefinite)")
                .def("build", &archXplore::system::AbstractSystem::build, "Build the system")
                .def("newProcess", &archXplore::system::AbstractSystem::newProcess, py::keep_alive<1, 2>(),
                     pybind11::return_value_policy::reference, "Create a new process")
                .def("getElapsedTime", &archXplore::system::AbstractSystem::getElapsedTime, "Get the elapsed time of the system")
                .def_readwrite("interval", &archXplore::system::AbstractSystem::m_multithread_interval,
                               "Multithreading interval (in ticks)")
                .def_readonly("MAX_INTERVAL", &archXplore::system::AbstractSystem::MAX_INTERVAL, "Maximum multithreading interval (in ticks)");
                
            // Bind QemuSystem
            pybind11::class_<archXplore::system::qemu::QemuSystem, archXplore::system::AbstractSystem>(system, "QemuSystem", pybind11::dynamic_attr())
                .def(pybind11::init<>());
        };
    } // namespace python

} // namespace archXplore
