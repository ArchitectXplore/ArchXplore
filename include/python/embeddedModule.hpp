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

#include "python/pySpartaLogTap.hpp"

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

            static pybind11::module_ &createSubPackage(pybind11::module_ &parent, const std::string &name)
            {
                if (m_bindPackagePool.find(name) == m_bindPackagePool.end())
                {
                    auto m = parent.def_submodule(name.c_str());
                    m_bindPackagePool[name] = &m;
                }
                assert(m_bindPackagePool[name] != nullptr);
                return *m_bindPackagePool[name];
            };
            embeddedModule(funcPtrType func)
            {
                getFuncPool().push_back(func);
            };
            ~embeddedModule(){};

        private:
            static std::shared_ptr<std::vector<funcPtrType>> m_bindFuncPool;
            static std::unordered_map<std::string, pybind11::module_ *> m_bindPackagePool;
        };

    } // namespace python

} // namespace archXplore

#define REGISTER_SPARTA_UNIT(UnitClass, ParamClass)                                                                                       \
    using UnitClass##Factory = sparta::ResourceFactory<UnitClass, ParamClass>;                                                            \
    class UnitClass##Component : public sparta::ResourceTreeNode                                                                          \
    {                                                                                                                                     \
    public:                                                                                                                               \
        UnitClass##Component(TreeNode *parent)                                                                                            \
            : ResourceTreeNode(parent, #UnitClass "Component", "Resource Tree Node of" #UnitClass "Component", new UnitClass##Factory){}; \
        UnitClass##Component()                                                                                                            \
            : ResourceTreeNode(#UnitClass "Component", "Resource Tree Node of" #UnitClass "Component", new UnitClass##Factory){};         \
        ~UnitClass##Component(){};                                                                                                        \
    };                                                                                                                                    \
    class UnitClass##Ports                                                                                                                \
    {                                                                                                                                     \
    public:                                                                                                                               \
        UnitClass##Ports(sparta::PortSet *ptr) : m_ports(ptr){};                                                                          \
        ~UnitClass##Ports(){};                                                                                                            \
        sparta::Port *getPort(const std::string &name)                                                                                    \
        {                                                                                                                                 \
            return m_ports->getPort(name);                                                                                                \
        };                                                                                                                                \
        sparta::PortVec *getPortVec(const std::string &name)                                                                              \
        {                                                                                                                                 \
            return m_ports->getPortVec(name);                                                                                             \
        }                                                                                                                                 \
                                                                                                                                          \
    private:                                                                                                                              \
        sparta::PortSet *m_ports;                                                                                                         \
    };                                                                                                                                    \
    void bind_##UnitClass(pybind11::module_ &m)                                                                                           \
    {                                                                                                                                     \
        auto unitBind = pybind11::class_<UnitClass>(m, "__" #UnitClass, pybind11::dynamic_attr());                                        \
        auto paramBind = pybind11::class_<ParamClass>(m, "__" #ParamClass, pybind11::dynamic_attr());                                     \
        auto portBind = pybind11::class_<UnitClass##Ports>(m, "__" #UnitClass "Ports", pybind11::dynamic_attr());                         \
        auto factoryBind = pybind11::class_<UnitClass##Factory>(m, "__" #UnitClass "Factory", pybind11::dynamic_attr());                  \
        auto componentBind = pybind11::class_<UnitClass##Component>(m, #UnitClass,                                                        \
                                                                    pybind11::dynamic_attr())                                             \
                                 .def(pybind11::init<sparta::TreeNode *>())                                                               \
                                 .def(pybind11::init<>())                                                                                 \
                                 .def(                                                                                                    \
                                     "attachTap",                                                                                         \
                                     [](UnitClass##Component &self, const std::string *category, pybind11::object &dest)                  \
                                     {                                                                                                    \
                                         return new sparta::log::PyTap(self.getResourceNow()->getContainer(), category, dest);            \
                                     },                                                                                                   \
                                     pybind11::return_value_policy::reference)                                                            \
                                 .def(                                                                                                    \
                                     "attachTap",                                                                                         \
                                     [](UnitClass##Component &self, const std::string *category, std::string &dest)                       \
                                     {                                                                                                    \
                                         return new sparta::log::PyTap(self.getResourceNow()->getContainer(), category, dest);            \
                                     },                                                                                                   \
                                     pybind11::return_value_policy::reference);                                                           \
                                                                                                                                          \
        componentBind.def_property_readonly(                                                                                              \
            "Params",                                                                                                                     \
            [](UnitClass##Component &self) {                                                                                              \
                return static_cast<ParamClass *>(self.getParameterSet());                                                                 \
            },                                                                                                                            \
            pybind11::return_value_policy::reference);                                                                                    \
                                                                                                                                          \
        componentBind.def_property_readonly(                                                                                              \
            "Ports",                                                                                                                      \
            [](UnitClass##Component &self) {                                                                                              \
                UnitClass##Ports *myPorts = new UnitClass##Ports(self.getResourceAs<sparta::Unit>()->getPortSet());                       \
                return myPorts;                                                                                                           \
            },                                                                                                                            \
            pybind11::return_value_policy::reference);                                                                                    \
                                                                                                                                          \
        sparta::RootTreeNode rtn;                                                                                                         \
        UnitClass##Component curTn(&rtn);                                                                                                 \
        sparta::Scheduler scheduler;                                                                                                      \
        sparta::Clock clk("clk", &scheduler);                                                                                             \
        rtn.setClock(&clk);                                                                                                               \
        rtn.enterConfiguring();                                                                                                           \
        rtn.enterFinalized();                                                                                                             \
                                                                                                                                          \
        sparta::TreeNode::ChildrenVector paramVec = curTn.getParameterSet()->getChildren();                                               \
        for (auto it = paramVec.begin(); it != paramVec.end(); ++it)                                                                      \
        {                                                                                                                                 \
            const std::string &name = (*it)->getName();                                                                                   \
            paramBind.def_property(                                                                                                       \
                name.c_str(),                                                                                                             \
                [=](ParamClass &self) {                                                                                                   \
                    return self.getParameter(name);                                                                                       \
                },                                                                                                                        \
                [=](ParamClass &self, const pybind11::object &value) {                                                                    \
                    self.getParameter(name)->pyset(value);                                                                                \
                },                                                                                                                        \
                pybind11::return_value_policy::reference);                                                                                \
        }                                                                                                                                 \
        sparta::PortSet *portSet = curTn.getResourceAs<sparta::Unit>()->getPortSet();                                                     \
        sparta::PortSet::RegisteredPortMap portMap;                                                                                       \
        portMap = portSet->getPorts(sparta::Port::Direction::IN);                                                                         \
        for (auto it = portMap.begin(); it != portMap.end(); it++)                                                                        \
        {                                                                                                                                 \
            const std::string &name = it->first;                                                                                          \
            portBind.def_property_readonly(                                                                                               \
                name.c_str(),                                                                                                             \
                [=](UnitClass##Ports &self) {                                                                                             \
                    return self.getPort(name);                                                                                            \
                },                                                                                                                        \
                pybind11::return_value_policy::reference);                                                                                \
        }                                                                                                                                 \
        portMap = portSet->getPorts(sparta::Port::Direction::OUT);                                                                        \
        for (auto it = portMap.begin(); it != portMap.end(); it++)                                                                        \
        {                                                                                                                                 \
            const std::string &name = it->first;                                                                                          \
            portBind.def_property_readonly(                                                                                               \
                name.c_str(),                                                                                                             \
                [=](UnitClass##Ports &self) {                                                                                             \
                    return self.getPort(name);                                                                                            \
                },                                                                                                                        \
                pybind11::return_value_policy::reference);                                                                                \
        }                                                                                                                                 \
        sparta::PortSet::RegisteredPortVecMap portVecMap;                                                                                 \
        portVecMap = portSet->getPortVecs(sparta::Port::Direction::IN);                                                                   \
        for (auto it = portVecMap.begin(); it != portVecMap.end(); it++)                                                                  \
        {                                                                                                                                 \
            const std::string &name = it->first;                                                                                          \
            portBind.def_property_readonly(                                                                                               \
                name.c_str(),                                                                                                             \
                [=](UnitClass##Ports &self) {                                                                                             \
                    return self.getPortVec(name);                                                                                         \
                },                                                                                                                        \
                pybind11::return_value_policy::reference);                                                                                \
        }                                                                                                                                 \
        portVecMap = portSet->getPortVecs(sparta::Port::Direction::OUT);                                                                  \
        for (auto it = portVecMap.begin(); it != portVecMap.end(); it++)                                                                  \
        {                                                                                                                                 \
            const std::string &name = it->first;                                                                                          \
            portBind.def_property_readonly(                                                                                               \
                name.c_str(),                                                                                                             \
                [=](UnitClass##Ports &self) {                                                                                             \
                    return self.getPortVec(name);                                                                                         \
                },                                                                                                                        \
                pybind11::return_value_policy::reference);                                                                                \
        }                                                                                                                                 \
                                                                                                                                          \
        rtn.enterTeardown();                                                                                                              \
    }                                                                                                                                     \
                                                                                                                                          \
    archXplore::python::embeddedModule embedded_##UnitClass(&bind_##UnitClass)
