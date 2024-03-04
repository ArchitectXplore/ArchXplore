
#include <sparta/sparta.hpp>
#include <sparta/simulation/Unit.hpp>
#include <sparta/simulation/TreeNode.hpp>
#include <sparta/simulation/TreeNodePrivateAttorney.hpp>
#include <sparta/ports/PortSet.hpp>
#include <sparta/ports/PortVec.hpp>
#include <sparta/ports/DataPort.hpp>
#include <sparta/ports/Port.hpp>
#include <sparta/log/Tap.hpp>

#include "python/embeddedModule.hpp"

namespace sparta
{

    namespace py = pybind11;

    void bindSpartaModules(pybind11::module_ &parent)
    {
        // py::dynamic_attr() means we can add dynamic
        // variables to TreeNode in python

        auto m = archXplore::python::embeddedModule::createSubPackage(parent, "sparta");

        auto TreeNodeBind = py::class_<TreeNode>(m, "TreeNode", py::dynamic_attr());

        py::class_<log::PyTap>(m, "PyLogTap")
            .def(py::init<TreeNode *, const std::string *, py::object &>())
            .def("detach", &log::PyTap::detach)
            .def("reset", &log::PyTap::reset);

        py::class_<Scheduler>(m, "Scheduler")
            .def(py::init<>())
            .def("finalize", &Scheduler::finalize)
            .def("run",
                 [](Scheduler &self, Scheduler::Tick num_ticks)
                 {
                     self.run(num_ticks);
                 })
            .def("run",
                 [](Scheduler &self)
                 {
                     self.run();
                 });

        py::class_<Clock, TreeNode>(m, "Clock")
            .def(py::init<const std::string &, Scheduler *>());

        py::enum_<Port::Direction>(m, "PortDirection", py::arithmetic())
            .value("IN", Port::Direction::IN)
            .value("OUT", Port::Direction::OUT);

        py::enum_<SchedulingPhase>(m, "SchedulingPhase", py::arithmetic())
            .value("Update", SchedulingPhase::Update)         //!< Resources are updated in this phase
            .value("PortUpdate", SchedulingPhase::PortUpdate) //!< N-cycle Ports are updated in this phase
            .value("Flush", SchedulingPhase::Flush)           //!< Phase where flushing of pipelines, etc can occur
            .value("Collection", SchedulingPhase::Collection) //!< Pipeline collection occurs here
            .value("Tick", SchedulingPhase::Tick)             //!< Most operations (combinational logic) occurs in this phase
            .value("PostTick", SchedulingPhase::PostTick)     //!< Operations such as post-tick pipeline collection occur here
            ;

        py::class_<Port, TreeNode>(m, "Port")
            .def("bind",
                 [](Port &self, Port *that)
                 {
                     return self.bind(that);
                 })
            .def("__lshift__",
                 [](Port &self, Port *that)
                 {
                     return self.bind(that);
                 })
            .def("__rshift__",
                 [](Port &self, Port *that)
                 {
                     return self.bind(that);
                 });

        py::class_<PortSet, TreeNode>(m, "PortSet", py::dynamic_attr())
            .def(py::init<TreeNode *, const std::string &>())
            .def("getPort", &PortSet::getPort, py::return_value_policy::reference);

        py::class_<PortVec, TreeNode>(m, "PortVec", py::dynamic_attr())
            .def(py::init<TreeNode *, const std::string &, const Port::Direction, const std::string &>())
            .def("getPort", &PortVec::getPort, py::return_value_policy::reference)
            .def(
                "__getitem__",
                [](PortVec &self, uint32_t index)
                {
                    return self.getPort(index);
                },
                py::return_value_policy::reference);

        py::class_<InPort, Port>(m, "InPort")
            .def("bind",
                 [](InPort &self, Port *out)
                 {
                     self.bind(out);
                 })
            .def("__lshift__",
                 [](InPort &self, Port *out)
                 {
                     self.bind(out);
                 })
            .def("__rshift__",
                 [](InPort &self, Port *out)
                 {
                     self.bind(out);
                 });

        py::class_<OutPort, Port>(m, "OutPort")
            .def(py::init<TreeNode *, const std::string &, bool>())
            .def("bind",
                 [](OutPort &self, Port *in)
                 {
                     self.bind(in);
                 })
            .def("__lshift__",
                 [](OutPort &self, Port *in)
                 {
                     self.bind(in);
                 })
            .def("__rshift__",
                 [](OutPort &self, Port *in)
                 {
                     self.bind(in);
                 });

        py::class_<DataOutPort<uint32_t>, OutPort>(m, "DataOutPort_uint32")
            .def(py::init<TreeNode *, const std::string &, bool>())
            .def(py::init<TreeNode *, const std::string &>())
            .def("send", &DataOutPort<uint32_t>::send)
            .def("send",
                 [](DataOutPort<uint32_t> &self, uint32_t dat)
                 {
                     return self.send(dat);
                 });

        py::class_<ParameterBase, TreeNode>(m, "ParameterBase")
            .def("set", &ParameterBase::pyset)
            .def("__eq__",
                 [](ParameterBase &self, const py::object &val)
                 {
                     self.pyset(val);
                 })
            .def("getValueAsString", &ParameterBase::getValueAsString)
            .def("__str__",
                 [](ParameterBase &self)
                 {
                     return self.getValueAsString();
                 })
            .def("getTypeName", &ParameterBase::getTypeName);

        py::class_<ParameterSet, TreeNode>(m, "ParameterSet", py::dynamic_attr())
            .def(py::init<TreeNode *>())
            .def(
                "getParameter",
                [](const ParameterSet &self, const std::string &name)
                {
                    return self.getChildAs<ParameterBase>(name, true);
                },
                py::return_value_policy::reference);

        TreeNodeBind
            // TreeNode constructor with no is_indexable parameter
            .def(py::init<TreeNode *, const std::string &,
                          const std::string &, TreeNode::group_idx_type, const std::string &>())
            .def(py::init<TreeNode *, const std::string &,
                          const std::string &>())
            //.def("getParent", &TreeNode::getParent);
            // .def(
            //     "getChildren",
            //     [](const TreeNode &self)
            //     {
            //         return self.getChildren();
            //     },
            //     py::return_value_policy::reference)
            // .def(
            //     "getAllChildren",
            //     [](const TreeNode &self)
            //     {
            //         return TreeNodePrivateAttorney::getAllChildren(self);
            //     },
            //     py::return_value_policy::reference)
            .def(
                "getName",
                [](const TreeNode &self)
                {
                    return self.getName();
                },
                py::return_value_policy::reference)
            .def(
                "getDesc",
                [](const TreeNode &self)
                {
                    return self.getDesc();
                },
                py::return_value_policy::reference)
            // py::return_value_policy::reference_internal);
            // .def("setClock",
            //      [](TreeNode &self, const Clock *clk)
            //      {
            //          return self.setClock(clk);
            //      })
            // .def(
            //     "asPortSet",
            //     [](TreeNode &self)
            //     {
            //         return self.getAs<PortSet>();
            //     },
            //     py::return_value_policy::reference)
            // .def(
            //     "asParamSet",
            //     [](TreeNode &self)
            //     {
            //         return self.getAs<ParameterSet>();
            //     },
            //     py::return_value_policy::reference)
            // // getChild Part
            // .def(
            //     "getChildAsPortSet",
            //     [](TreeNode &self, const std::string &name)
            //     {
            //         return self.getChildAs<PortSet>(name);
            //     },
            //     py::return_value_policy::reference)
            // .def(
            //     "getChildAsPort",
            //     [](TreeNode &self, const std::string &name)
            //     {
            //         return self.getChildAs<Port>(name);
            //     },
            //     py::return_value_policy::reference)
            // // ParamSet
            // .def(
            //     "getChildAsParamSet",
            //     [](TreeNode &self, const std::string &name)
            //     {
            //         return self.getChildAs<ParameterSet>(name);
            //     },
            //     py::return_value_policy::reference)
            .def(
                "attachTap",
                [](TreeNode &self, const std::string *category, py::object &dest)
                {
                    return new log::PyTap(&self, category, dest);
                },
                py::return_value_policy::reference);

        py::class_<GlobalTreeNode, TreeNode>(m, "GlobalTreeNode", py::dynamic_attr())
            .def(py::init<>());

        py::class_<RootTreeNode, TreeNode>(m, "RootTreeNode", py::dynamic_attr())
            .def(py::init<const std::string &, const std::string &, GlobalTreeNode *>())
            .def(py::init<const std::string &, const std::string &>())
            .def(py::init<const std::string &>())
            .def(py::init<>())
            .def("enterTeardown", &RootTreeNode::enterTeardown)
            .def("bindTreeEarly", &RootTreeNode::bindTreeEarly)
            .def("bindTreeLate", &RootTreeNode::bindTreeLate)
            .def("enterConfiguring", &RootTreeNode::enterConfiguring)
            .def("enterFinalized",
                 [](RootTreeNode &self)
                 {
                     self.enterFinalized();
                 });

        py::class_<ResourceTreeNode, TreeNode>(m, "ResourceTreeNode", py::dynamic_attr())
            .def(
                "getParameterSet",
                [](ResourceTreeNode &self)
                {
                    return self.getParameterSet();
                },
                py::return_value_policy::reference)
            .def(
                "getPortSet",
                [](ResourceTreeNode &self)
                {
                    Unit *unit = self.getResourceAs<Unit *>();
                    return unit->getPortSet();
                },
                py::return_value_policy::reference);

    }
}
