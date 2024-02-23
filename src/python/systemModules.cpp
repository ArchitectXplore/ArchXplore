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
                .def("run", [](abstractSystem& self){self.run(-1);})
                .def("build", &abstractSystem::build)
                ;

            pybind11::class_<qemu::qemuSystem, abstractSystem>(system, "qemuSystem", pybind11::dynamic_attr())
                .def(pybind11::init<>())
                .def("boot",
                     [](qemu::qemuSystem &self, const pybind11::object &pyargv11)
                     {
                         int argc = 0;
                         char **argv;

                         // convert input list to C/C++ argc/argv
                         PyObject *pyargv = pyargv11.ptr();
                         if (PySequence_Check(pyargv))
                         {
                             Py_ssize_t sz = PySequence_Size(pyargv);
                             argc = (int)sz;
                             argv = new char *[sz];
                             for (Py_ssize_t i = 0; i < sz; ++i)
                             {
                                 PyObject *item = PySequence_GetItem(pyargv, i);
                                 argv[i] = (char *)PyUnicode_AsUTF8(item);
                                 Py_DECREF(item);
                                 if (!argv[i] || PyErr_Occurred())
                                 {
                                     argv = nullptr;
                                     break;
                                 }
                             }
                         }
                         // bail if failed to convert
                         if (!argv)
                         {
                             std::cerr << "argument is not a sequence of strings" << std::endl;
                         }
                         self.boot({argc, argv, nullptr});
                     });
        };
        python::embeddedModule embedded_system(&bind_system_func);

    } // namespace system

} // namespace archXplore
