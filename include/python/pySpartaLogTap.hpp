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
            PyTap(TreeNode *node, const std::string *pcategory, py::object &dest)
            {
                myCategory = new std::string(*pcategory);
                try {
                    std::string t = dest.cast<std::string>();
                    myTap = new Tap(node, myCategory, t);
                }
                catch(...){
                    myBuffer = new py::detail::pythonbuf(dest);
                    myStream = new std::ostream(myBuffer);
                    myTap = new Tap(node, myCategory, *myStream);
                }
            };
            ~PyTap()
            {
                if(myTap != nullptr) delete myTap;
                if(myStream != nullptr) delete myStream;
                if(myBuffer != nullptr) delete myBuffer;
                if(myCategory != nullptr) delete myCategory;
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
            std::ostream* myStream;
            py::detail::pythonbuf* myBuffer;
            std::string* myCategory;
        };
    }
}
