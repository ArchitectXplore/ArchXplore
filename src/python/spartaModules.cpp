#include "python/embeddedModule.hpp"
#include <sparta/sparta.hpp>
#include <sparta/simulation/TreeNode.hpp>
#include <sparta/simulation/TreeNodePrivateAttorney.hpp>
#include <sparta/ports/PortSet.hpp>
#include <sparta/ports/DataPort.hpp>
#include <sparta/ports/Port.hpp>
#include <sparta/log/Tap.hpp>
#include "testpointer.cpp"

// #define GPC_DEV

namespace sparta { 

namespace py = pybind11;

namespace log {
    class PyTap {
    public:
        PyTap(TreeNode* node, const std::string* pcategory, std::string& dest) 
        : myBuffer(py::module_::import("sys").attr("stdout")), myStream(nullptr){
            myTap = new Tap(node, pcategory, dest);
        };
        PyTap(TreeNode* node, const std::string* pcategory, py::object& dest) 
        : myBuffer(dest), myStream(&myBuffer){
            myTap = new Tap(node, pcategory, myStream);
        };
        ~PyTap(){
            delete myTap;
        };
        void detach() {
            myTap->detach();
        };
        void reset(TreeNode * node) {
            myTap->reset(node);
        }

    protected : 
        Tap* myTap;
        std::ostream myStream;
        py::detail::pythonbuf myBuffer;
    };
}

void bindSpartaModules(pybind11::module_& parent){
    //py::dynamic_attr() means we can add dynamic
    //variables to TreeNode in python

    auto m  = parent.def_submodule("sparta");

    py::class_<sparta::log::Tap>(m, "LogTap", py::dynamic_attr())
        .def(py::init<TreeNode* , const std::string* , std::ostream& >())
        .def(py::init<TreeNode* , const std::string* , std::string& >())
        .def("detach", &sparta::log::Tap::detach)
        .def("reset", &sparta::log::Tap::reset)
        ;

    py::class_<sparta::log::PyTap>(m, "PyLogTap", py::dynamic_attr())
        .def(py::init<TreeNode* , const std::string* , std::string& >())
        .def(py::init<TreeNode* , const std::string* , py::object& >())
        .def("detach", &sparta::log::PyTap::detach)
        .def("reset", &sparta::log::PyTap::reset)
        ;

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
        //getChild Part
        .def("getChildAsPortSet", \
            [](TreeNode& self, const std::string& name) { \
                return self.getChildAs<PortSet>(name); \
            },  py::return_value_policy::reference)
        .def("getChildAsPort", \
            [](TreeNode& self, const std::string& name) { \
                return self.getChildAs<Port>(name); \
            },  py::return_value_policy::reference)
        ;

    py::class_<sparta::RootTreeNode,sparta::TreeNode>(m, "RootTreeNode")
        .def(py::init<const std::string &, const std::string &, GlobalTreeNode* >())
        .def(py::init<const std::string &, const std::string &>())
        .def(py::init<const std::string &>())
        .def(py::init<>())
        .def("enterTeardown", &sparta::RootTreeNode::enterTeardown)
        .def("bindTreeEarly", &sparta::RootTreeNode::bindTreeEarly)
        .def("bindTreeLate", &sparta::RootTreeNode::bindTreeLate)
        .def("enterConfiguring", &sparta::RootTreeNode::enterConfiguring)
        .def("enterFinalized", 
            [](RootTreeNode& self) {
                self.enterFinalized();
            }
        )
        .def("setClock", 
            [](RootTreeNode& self, const Clock * clk) {
                self.setClock(clk);
            }
        )
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

    m.attr("Tick") = py::int_(sizeof(sparta::Scheduler::Tick) * CHAR_BIT);

    py::class_<Clock, TreeNode>(m, "Clock")
        .def(py::init<const std::string&, Scheduler*>());

    py::class_<PortSet, TreeNode>(m, "PortSet")
        .def(py::init<TreeNode *, const std::string &>());
    
    py::class_<ParameterSet, TreeNode>(m, "ParameterSet")
        .def(py::init<TreeNode *>());
    
    py::enum_<Port::Direction>(m, "PortDirection", py::arithmetic())
        .value("IN", Port::Direction::IN)
        .value("OUT", Port::Direction::OUT)
        ;
        
    py::enum_<SchedulingPhase>(m, "SchedulingPhase", py::arithmetic())
        .value("Update", SchedulingPhase::Update)           //!< Resources are updated in this phase
        .value("PortUpdate", SchedulingPhase::PortUpdate)   //!< N-cycle Ports are updated in this phase
        .value("Flush", SchedulingPhase::Flush)             //!< Phase where flushing of pipelines, etc can occur
        .value("Collection", SchedulingPhase::Collection)   //!< Pipeline collection occurs here
        .value("Tick", SchedulingPhase::Tick)               //!< Most operations (combinational logic) occurs in this phase
        .value("PostTick", SchedulingPhase::PostTick)       //!< Operations such as post-tick pipeline collection occur here
        ;

    py::class_<Port, TreeNode>(m, "Port")
        .def("bind",
            [](Port& self, Port* that){
                return self.bind(that);
            })
        .def("__lshift__",
            [](Port& self, Port * that){
                return self.bind(that);
            })
        .def("__rshift__",
            [](Port& self, Port * that){
                return self.bind(that);
            });

    py::class_<InPort, Port>(m, "InPort")
        .def("bind",
            [](InPort &self, Port* out){
                return self.bind(out);
            })
        .def("__lshift__",
            [](InPort& self, Port * out){
                return self.bind(out);
            })
        .def("__rshift__",
            [](InPort& self, Port * out){
                return self.bind(out);
            });

    py::class_<OutPort, Port>(m, "OutPort")
        .def(py::init<TreeNode *, const std::string&, bool >())
        .def("bind",
            [](OutPort &self, Port* in){
                return self.bind(in);
            })
        .def("__lshift__",
            [](OutPort& self, Port * in){
                return self.bind(in);
            })
        .def("__rshift__",
            [](OutPort& self, Port * in){
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

    py::class_<ParameterBase, TreeNode>(m, "ParameterBase")
        ;

#ifdef GPC_DEV

    py::class_<Parameter<uint32_t>, ParameterBase>(m, "ParameterU32")
        .def("getTypeName", &Parameter<uint32_t>::getTypeName)
        .def("getDefault", &Parameter<uint32_t>::getDefault)
        .def("getDefaultAsString", &Parameter<uint32_t>::getDefaultAsString)
        .def("getValue", &Parameter<uint32_t>::getValue)
        .def("getValueAsString", &Parameter<uint32_t>::getValueAsString)
        .def("peekValue", &Parameter<uint32_t>::peekValue)
        .def("ignore", &Parameter<uint32_t>::ignore)
        .def("unread", &Parameter<uint32_t>::unread)
        .def("isVector", &Parameter<uint32_t>::isVector)
        .def("isVisibilityAllowed", &Parameter<uint32_t>::isVisibilityAllowed)
        // .def(py::self == py::self)
        // .def(py::self != py::self)
        // .def(py::self > py::self)
        // .def(py::self >= py::self)
        // .def(py::self <= py::self)
        // .def(py::self = py::self)
        .def("__eq__", [](Parameter<uint32_t>& p, uint32_t a){
            p = a; 
        }, py::is_operator())
        ;   

    py::class_<Parameter<bool>, ParameterBase>(m, "ParameterBool")
        .def("getTypeName", &Parameter<bool>::getTypeName)
        .def("getDefault", &Parameter<bool>::getDefault)
        .def("getDefaultAsString", &Parameter<bool>::getDefaultAsString)
        .def("getValue", &Parameter<bool>::getValue)
        .def("getValueAsString", &Parameter<bool>::getValueAsString)
        .def("peekValue", &Parameter<bool>::peekValue)
        .def("ignore", &Parameter<bool>::ignore)
        .def("unread", &Parameter<bool>::unread)
        .def("isVector", &Parameter<bool>::isVector)
        .def("isVisibilityAllowed", &Parameter<bool>::isVisibilityAllowed)
        // .def(py::self == py::self)
        // .def(py::self != py::self)
        // .def(py::self > py::self)
        // .def(py::self >= py::self)
        // .def(py::self <= py::self)
        // .def(py::self = py::self)
        .def("__eq__", [](Parameter<bool>& p, bool a){
            p = a; 
        }, py::is_operator())
        ;   
        
#endif

}   

    archXplore::python::embeddedModule embedded_sparta(&bindSpartaModules);
}
