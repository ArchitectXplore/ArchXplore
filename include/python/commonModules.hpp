#include "python/embeddedModule.hpp"

namespace archXplore
{
    namespace python
    {

        void bindCommonModules(pybind11::module_ &parent)
        {
            // Bind ISS Modules
            pybind11::class_<clockedObject>(parent, "clockedObject")
                .def(pybind11::init<sparta::TreeNode *, const std::string &, const std::string &>())
                .def("attachTap", &clockedObject::attachTap, pybind11::return_value_policy::reference)
                .def("setClockDomain", &clockedObject::setClockDomain, pybind11::return_value_policy::reference)
                .def("setClockFrequency", &clockedObject::setClockFrequency, pybind11::return_value_policy::reference)
                .def("setRank", &clockedObject::setRank, pybind11::return_value_policy::reference)
                ;
        };
    } // namespace python

} // namespace archXplore
