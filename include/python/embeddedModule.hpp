#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include <sparta/sparta.hpp>
#include <sparta/ports/Port.hpp>
#include <sparta/ports/PortSet.hpp>
#include <sparta/ports/PortVec.hpp>
#include <sparta/log/Tap.hpp>
#include <sparta/statistics/StatisticSet.hpp>
#include <sparta/statistics/CounterBase.hpp>

#include "clockedObject.hpp"

namespace archXplore
{

    namespace python
    {

        class embeddedModule
        {
        public:
            typedef std::function<void(pybind11::module_ &)> funcPtrType;

        public:
            static std::vector<funcPtrType> &getFuncPool()
            {
                if (m_bindFuncPool == nullptr)
                {
                    m_bindFuncPool = std::make_shared<std::vector<funcPtrType>>();
                }
                return *m_bindFuncPool;
            };

            static pybind11::module_ createSubPackage(pybind11::module_ &parent, const std::string &name)
            {
                return parent.def_submodule(name.c_str());
            };
            embeddedModule(funcPtrType func)
            {
                getFuncPool().push_back(func);
            };
            ~embeddedModule(){};

        private:
            static std::shared_ptr<std::vector<funcPtrType>> m_bindFuncPool;
        };

    } // namespace python

} // namespace archXplore

#define REGISTER_SPARTA_UNIT(UnitClass, ParamClass)                                                                                     \
    using UnitClass##Factory = sparta::ResourceFactory<UnitClass, ParamClass>;                                                          \
    class UnitClass##Component : public archXplore::clockedObject                                                                       \
    {                                                                                                                                   \
    public:                                                                                                                             \
        UnitClass##Component(TreeNode *parent, const std::string &name)                                                                 \
            : clockedObject(parent, name, "Clocked Object of " #UnitClass),                                                             \
              m_resourceTreeNode(this, name, "Resource Tree Node of " #UnitClass, new UnitClass##Factory){};                            \
        ~UnitClass##Component(){};                                                                                                      \
        sparta::PortSet *getPortSet() { return m_resourceTreeNode.getResourceAs<sparta::Unit>()->getPortSet(); }                        \
        sparta::StatisticSet *getStatisticSet() { return m_resourceTreeNode.getResourceAs<sparta::Unit>()->getStatisticSet(); }         \
        sparta::ParameterSet *getParameterSet() { return m_resourceTreeNode.getParameterSet(); }                                        \
                                                                                                                                        \
    private:                                                                                                                            \
        sparta::ResourceTreeNode m_resourceTreeNode;                                                                                    \
    };                                                                                                                                  \
    class UnitClass##Statistics                                                                                                         \
    {                                                                                                                                   \
    public:                                                                                                                             \
        UnitClass##Statistics(sparta::StatisticSet*ptr) : m_statistics(ptr){};                                                          \
        ~UnitClass##Statistics(){};                                                                                                     \
        sparta::CounterBase *getCounter(const std::string &name)                                                                        \
        {                                                                                                                               \
            return m_statistics->getCounter(name);                                                                                      \
        };                                                                                                                              \
        const sparta::StatisticSet::CounterVector getCounters()                                                                         \
        {                                                                                                                               \
            return m_statistics->getCounters();                                                                                         \
        }                                                                                                                               \
                                                                                                                                        \
    private:                                                                                                                            \
        sparta::StatisticSet *m_statistics;                                                                                             \
    };                                                                                                                                  \
    class UnitClass##Ports                                                                                                              \
    {                                                                                                                                   \
    public:                                                                                                                             \
        UnitClass##Ports(sparta::PortSet *ptr) : m_ports(ptr){};                                                                        \
        ~UnitClass##Ports(){};                                                                                                          \
        sparta::Port *getPort(const std::string &name)                                                                                  \
        {                                                                                                                               \
            return m_ports->getPort(name);                                                                                              \
        };                                                                                                                              \
        sparta::PortVec *getPortVec(const std::string &name)                                                                            \
        {                                                                                                                               \
            return m_ports->getPortVec(name);                                                                                           \
        }                                                                                                                               \
                                                                                                                                        \
    private:                                                                                                                            \
        sparta::PortSet *m_ports;                                                                                                       \
    };                                                                                                                                  \
    void bind_##UnitClass(pybind11::module_ &m)                                                                                         \
    {                                                                                                                                   \
        auto unitBind = pybind11::class_<UnitClass>(m, "__" #UnitClass, pybind11::dynamic_attr());                                      \
        auto paramBind = pybind11::class_<ParamClass>(m, "__" #ParamClass, pybind11::dynamic_attr());                                   \
        auto portBind = pybind11::class_<UnitClass##Ports>(m, "__" #UnitClass "Ports", pybind11::dynamic_attr());                       \
        auto statisticBind = pybind11::class_<UnitClass##Statistics>(m, "__" #UnitClass "Statistics", pybind11::dynamic_attr());        \
        auto factoryBind = pybind11::class_<UnitClass##Factory>(m, "__" #UnitClass "Factory", pybind11::dynamic_attr());                \
        auto componentBind = pybind11::class_<UnitClass##Component, archXplore::clockedObject>(m, #UnitClass, pybind11::dynamic_attr()) \
                                 .def(pybind11::init<sparta::TreeNode *, const std::string &>());                                       \
                                                                                                                                        \
        componentBind.def_property_readonly(                                                                                            \
            "Params",                                                                                                                   \
            [](UnitClass##Component &self) {                                                                                            \
                return static_cast<ParamClass *>(self.getParameterSet());                                                               \
            },                                                                                                                          \
            pybind11::return_value_policy::reference);                                                                                  \
                                                                                                                                        \
        componentBind.def_property_readonly(                                                                                            \
            "Ports",                                                                                                                    \
            [](UnitClass##Component &self) {                                                                                            \
                UnitClass##Ports *myPorts = new UnitClass##Ports(self.getPortSet());                                                    \
                return myPorts;                                                                                                         \
            },                                                                                                                          \
            pybind11::return_value_policy::reference);                                                                                  \
                                                                                                                                        \
        componentBind.def_property_readonly(                                                                                            \
            "Statistics",                                                                                                               \
            [](UnitClass##Component &self) {                                                                                            \
                UnitClass##Statistics *myStatistics = new UnitClass##Statistics(self.getStatisticSet());                                \
                return myStatistics;                                                                                                    \
            },                                                                                                                          \
            pybind11::return_value_policy::reference);                                                                                  \
                                                                                                                                        \
        sparta::RootTreeNode rtn;                                                                                                       \
        UnitClass##Component curTn(&rtn, #UnitClass);                                                                                   \
        sparta::Scheduler scheduler;                                                                                                    \
        sparta::Clock clk("clk", &scheduler);                                                                                           \
        rtn.setClock(&clk);                                                                                                             \
        rtn.enterConfiguring();                                                                                                         \
        rtn.enterFinalized();                                                                                                           \
                                                                                                                                        \
        sparta::TreeNode::ChildrenVector paramVec = curTn.getParameterSet()->getChildren();                                             \
        for (auto it = paramVec.begin(); it != paramVec.end(); ++it)                                                                    \
        {                                                                                                                               \
            const std::string &name = (*it)->getName();                                                                                 \
            paramBind.def_property(                                                                                                     \
                name.c_str(),                                                                                                           \
                [=](ParamClass &self) {                                                                                                 \
                    return self.getParameter(name);                                                                                     \
                },                                                                                                                      \
                [=](ParamClass &self, const pybind11::object &value) {                                                                  \
                    self.getParameter(name)->pyset(value);                                                                              \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        sparta::TreeNode::ChildrenVector statVec = curTn.getStatisticSet()->getChildren();                                              \
        for (auto it = statVec.begin(); it != statVec.end(); ++it){                                                                     \
            const std::string &name = (*it)->getName();                                                                                 \
            statisticBind.def_property_readonly(                                                                                        \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Statistics &self) {                                                                                      \
                    return self.getCounter(name);                                                                                       \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        sparta::PortSet *portSet = curTn.getPortSet();                                                                                  \
        sparta::PortSet::RegisteredPortMap portMap;                                                                                     \
        portMap = portSet->getPorts(sparta::Port::Direction::IN);                                                                       \
        for (auto it = portMap.begin(); it != portMap.end(); it++)                                                                      \
        {                                                                                                                               \
            const std::string &name = it->first;                                                                                        \
            portBind.def_property_readonly(                                                                                             \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Ports &self) {                                                                                           \
                    return self.getPort(name);                                                                                          \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        portMap = portSet->getPorts(sparta::Port::Direction::OUT);                                                                      \
        for (auto it = portMap.begin(); it != portMap.end(); it++)                                                                      \
        {                                                                                                                               \
            const std::string &name = it->first;                                                                                        \
            portBind.def_property_readonly(                                                                                             \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Ports &self) {                                                                                           \
                    return self.getPort(name);                                                                                          \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        sparta::PortSet::RegisteredPortVecMap portVecMap;                                                                               \
        portVecMap = portSet->getPortVecs(sparta::Port::Direction::IN);                                                                 \
        for (auto it = portVecMap.begin(); it != portVecMap.end(); it++)                                                                \
        {                                                                                                                               \
            const std::string &name = it->first;                                                                                        \
            portBind.def_property_readonly(                                                                                             \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Ports &self) {                                                                                           \
                    return self.getPortVec(name);                                                                                       \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        portVecMap = portSet->getPortVecs(sparta::Port::Direction::OUT);                                                                \
        for (auto it = portVecMap.begin(); it != portVecMap.end(); it++)                                                                \
        {                                                                                                                               \
            const std::string &name = it->first;                                                                                        \
            portBind.def_property_readonly(                                                                                             \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Ports &self) {                                                                                           \
                    return self.getPortVec(name);                                                                                       \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
                                                                                                                                        \
        rtn.enterTeardown();                                                                                                            \
    }                                                                                                                                   \
                                                                                                                                        \
    archXplore::python::embeddedModule embedded_##UnitClass(&bind_##UnitClass)
