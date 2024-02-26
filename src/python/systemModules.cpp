#include "python/embeddedModule.hpp"
#include "system/abstractSystem.hpp"
#include "system/qemu/qemuSystem.hpp"
// #include "system/qemu/qemuParallelSystem.hpp"

namespace archXplore
{

    namespace system
    {

        void bind_system_func(pybind11::module_ &parent)
        {
            // Bind System Modules
            auto system = python::embeddedModule::createSubPackage(parent, "system");

            pybind11::class_<abstractSystem, sparta::RootTreeNode>(system, "__abstractSystem", pybind11::dynamic_attr())
                .def("run", &abstractSystem::run)
                .def("run", [](abstractSystem &self)
                     { self.run(-1); })
                .def("build", &abstractSystem::build)
                .def_readwrite("workload", &abstractSystem::m_workload_path)
                .def_readwrite("interval", &abstractSystem::m_multithread_interval)
                ;

            pybind11::class_<qemu::qemuSystem, abstractSystem>(system, "qemuSystem", pybind11::dynamic_attr())
                .def(pybind11::init<>());
            // pybind11::class_<qemu::qemuParallelSystem, abstractSystem>(system, "qemuParallelSystem", pybind11::dynamic_attr())
            //     .def(pybind11::init<>())
            //     .def_readwrite("parallel_interval", &qemu::qemuParallelSystem::m_tick_quantum);
        };
        python::embeddedModule embedded_system(&bind_system_func);

    } // namespace system

} // namespace archXplore
