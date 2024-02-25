#include "python/embeddedModule.hpp"
#include "system/abstractSystem.hpp"
#include "system/qemu/qemuSystem.hpp"

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
                .def_readwrite("workload", &abstractSystem::m_workload_path);

            pybind11::class_<qemu::qemuSystem, abstractSystem>(system, "qemuSystem", pybind11::dynamic_attr())
                .def(pybind11::init<>());
        };
        python::embeddedModule embedded_system(&bind_system_func);

    } // namespace system

} // namespace archXplore
