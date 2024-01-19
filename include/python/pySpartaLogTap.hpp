#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include <sparta/log/Tap.hpp>

namespace sparta
{

    namespace py = pybind11;

    namespace log
    {
        class __attribute__((visibility("hidden"))) PyTap
        {
        public:
            PyTap(TreeNode *node, const std::string *pcategory, std::string &dest)
                : myBuffer(py::module_::import("sys").attr("stdout")), myStream(nullptr)
            {
                myTap = new Tap(node, pcategory, dest);
            };
            PyTap(TreeNode *node, const std::string *pcategory, py::object &dest)
                : myBuffer(dest), myStream(&myBuffer)
            {
                myTap = new Tap(node, pcategory, myStream);
            };
            ~PyTap()
            {
                delete myTap;
            };
            void detach()
            {
                myTap->detach();
            };
            void reset(TreeNode *node)
            {
                myTap->reset(node);
            }

        protected:
            Tap *myTap;
            std::ostream myStream;
            py::detail::pythonbuf myBuffer;
        };
    }
}
