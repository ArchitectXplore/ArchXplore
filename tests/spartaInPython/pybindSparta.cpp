#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sparta/sparta.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/TreeNodePrivateAttorney.hpp"
#include "sparta/ports/PortSet.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/Port.hpp"
namespace py = pybind11;
using namespace sparta;

#define SPARTA_DEF_getChildAs(ChildType) \
    .def("getChildAs" #ChildType, \
        [](TreeNode& self, const std::string& name) { \
            return self.getChildAs<ChildType>(name); \
        },  py::return_value_policy::reference)

// class PyPort : public Port{
// public:
//     /* Inherit the constructors */
//     using Port::Port;
//     /* Trampoline (need one for each virtual function) */
//     void bind(Port* out) override {
//         PYBIND11_OVERRIDE(
//             void, /* Return type */
//             Port,      /* Parent class */
//             bind,          /* Name of function in C++ (must match Python name) */
//             out         /* Argument(s) */
//         );
//     }
// };

PYBIND11_MODULE(pysparta, m){
    //py::dynamic_attr() means we can add dynamic
    //variables to TreeNode in python
    py::class_<sparta::TreeNode>(m, "TreeNode", py::dynamic_attr())
        //TreeNode constructor with no is_indexable parameter
        .def(py::init<sparta::TreeNode*, const std::string&, 
            const std::string&, sparta::TreeNode::group_idx_type, const std::string&>())
        .def(py::init<sparta::TreeNode*, const std::string&,
            const std::string&>() )
        //.def("getParent", &sparta::TreeNode::getParent);
        .def("getChildren", 
            [](const sparta::TreeNode& self) {
                return self.getChildren();
            },
            py::return_value_policy::reference)
        .def("getAllChildren",
            [](const TreeNode& self){
                return TreeNodePrivateAttorney::getAllChildren(self);
            },
            py::return_value_policy::reference)
        .def("getName", 
            [](const TreeNode& self){
                return self.getName();
            }, py::return_value_policy::reference)
            //py::return_value_policy::reference_internal);
        .def("setClock", 
            [](TreeNode& self, const Clock *clk){
                return self.setClock(clk);
            })
        .def("streamouttest",
            [](TreeNode &self){
                std::cout << "hello" << std::endl;
            })
        //getChild Part
        SPARTA_DEF_getChildAs(PortSet)
        SPARTA_DEF_getChildAs(Port)
        // SPARTA_DEF_GET_CHILD_AS(ParameterSet)
        // SPARTA_DEF_GET_CHILD_AS(ParameterBase)
        ;


    py::class_<sparta::Scheduler>(m, "Scheduler")
        .def(py::init<>() )
        .def("finalize", &Scheduler::finalize)
        .def("run", 
            [](Scheduler& self, Scheduler::Tick num_ticks){
                self.run(num_ticks);
            })
        .def("run", 
            [](Scheduler &self){
                self.run();
            })
        ;

    py::class_<Scheduler::Tick>(m, "Tick");

    py::class_<Clock, TreeNode>(m, "Clock")
        .def(py::init<const std::string&, Scheduler*>());

    py::class_<PortSet, TreeNode>(m, "PortSet")
        .def(py::init<TreeNode *, const std::string &>());
    
    py::class_<ParameterSet, TreeNode>(m, "ParameterSet")
        .def(py::init<TreeNode *>());
    
    py::enum_<Port::Direction>(m, "Port.Direction", py::arithmetic())
        .value("IN", Port::Direction::IN)
        .value("OUT", Port::Direction::OUT);

    //py::class_<Port, PyPort, TreeNode>(m, "Port");
    py::class_<Port, TreeNode>(m, "Port");
        // .def(py::init<TreeNode*, Port::Direction, const std::string &>() );

    py::class_<OutPort, Port>(m, "OutPort")
        .def(py::init<TreeNode *, const std::string&, bool >())
        .def("bind",
            [](OutPort &self, Port * in){
                return self.bind(in);
            });
    
    py::class_<DataOutPort<uint32_t>, OutPort>(m, "DataOutPort_uint32")
        .def(py::init<TreeNode *, const std::string &, bool >())
        .def(py::init<TreeNode *, const std::string &>())
        .def("send", &DataOutPort<uint32_t>::send)
        .def("send", 
            [](DataOutPort<uint32_t> &self, uint32_t dat){
                return self.send(dat);
            });

    
    m.def("bind", 
    [](Port * p1, Port *p2){
        bind(p1, p2);
    });
}   
