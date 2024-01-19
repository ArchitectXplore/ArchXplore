#include "python/embeddedModule.hpp"
#include "iss/abstractISS.hpp"
#include "iss/qemu/qemuISS.hpp"

namespace archXplore
{

    namespace iss
    {

        void bind_iss_func(pybind11::module_ &parent)
        {
            // Bind ISS Modules
            auto iss = python::embeddedModule::createSubPackage(parent, "iss");
            pybind11::class_<abstractISS>(iss, "__abstractISS");
            pybind11::class_<qemu::qemuISS, abstractISS>(iss, "qemuISS")
                .def(pybind11::init<>());
        }

        python::embeddedModule embedded_iss(&bind_iss_func);

    } // namespace iss

} // namespace archXplore
