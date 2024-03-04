#include "python/embeddedModule.hpp"

namespace archXplore
{
    namespace python
    {

        void bindCommonModules(pybind11::module_ &parent)
        {
            // Bind ISS Modules
            pybind11::class_<clockedObject, pyClockedObject, sparta::TreeNode>(parent, "clockedObject")
                .def(pybind11::init<sparta::TreeNode *, const std::string &>())
                .def("setClockDomain", &clockedObject::setClockDomain, pybind11::return_value_policy::reference)
                .def("setClockFrequency", &clockedObject::setClockFrequency, pybind11::return_value_policy::reference)
                .def("setRank", &clockedObject::setRank, pybind11::return_value_policy::reference)
                .def("buildTopology", &clockedObject::buildTopology);
        };
    } // namespace python

} // namespace archXplore
