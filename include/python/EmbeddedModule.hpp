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
#include <sparta/statistics/Histogram.hpp>

#include "ClockedObject.hpp"

namespace archXplore
{

    namespace python
    {

        class EmbeddedModule
        {
        public:
            typedef std::function<void(pybind11::module_ &)> FuncPtr_t;

        public:
            static std::vector<FuncPtr_t> &getFuncPool()
            {
                if (m_bindFuncPool == nullptr)
                {
                    m_bindFuncPool = std::make_shared<std::vector<FuncPtr_t>>();
                }
                return *m_bindFuncPool;
            };

            static pybind11::module_ createSubPackage(pybind11::module_ &parent, const std::string &name)
            {
                return parent.def_submodule(name.c_str());
            };
            EmbeddedModule(FuncPtr_t func)
            {
                getFuncPool().push_back(func);
            };
            ~EmbeddedModule(){};

        private:
            static std::shared_ptr<std::vector<FuncPtr_t>> m_bindFuncPool;
        };

    } // namespace python

} // namespace archXplore

#define REGISTER_SPARTA_UNIT(UnitClass, ParamClass)                                                                                     \
    using UnitClass##Factory = sparta::ResourceFactory<UnitClass, ParamClass>;                                                          \
    class UnitClass##Component : public archXplore::ClockedObject                                                                       \
    {                                                                                                                                   \
    public:                                                                                                                             \
        UnitClass##Component(TreeNode *parent, const std::string &name)                                                                 \
            : ClockedObject(parent, name),                                                                                              \
              m_resourceTreeNode(this, name, "Resource Tree Node of " #UnitClass, new UnitClass##Factory){};                            \
        ~UnitClass##Component(){};                                                                                                      \
        sparta::PortSet *getPortSet() { return m_resourceTreeNode.getResourceAs<sparta::Unit>()->getPortSet(); };                       \
        sparta::ParameterSet *getParameterSet() { return m_resourceTreeNode.getParameterSet(); };                                       \
        sparta::StatisticSet *getStatisticSet() { return m_resourceTreeNode.getResourceAs<sparta::Unit>()->getStatisticSet(); }         \
        void buildTopology() override{};                                                                                                \
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
        sparta::HistogramTreeNode *getHistogram(const std::string &name)                                                                        \
        {                                                                                                                               \
            return m_statistics->getHistogram(name);                                                                                      \
        };                                                                                                                              \
        const sparta::StatisticSet::HistogramTnVector getHistograms()                                                                         \
        {                                                                                                                               \
            return m_statistics->getHistograms();                                                                                         \
        }                                                                                                                                                                                 \
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
        auto componentBind = pybind11::class_<UnitClass##Component, archXplore::ClockedObject>(m, #UnitClass, pybind11::dynamic_attr()) \
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
        sparta::StatisticSet::CounterVector counterVec= curTn.getStatisticSet()->getCounters();                                              \
        for (auto it = counterVec.begin(); it != counterVec.end(); ++it){                                                                     \
            const std::string &name = (*it)->getName();                                                                                 \
            statisticBind.def_property_readonly(                                                                                        \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Statistics &self) {                                                                                      \
                    sparta::CounterBase *cntr = self.getCounter(name);                                                                  \
                    return cntr->get();                                                                                                 \
                },                                                                                                                      \
                pybind11::return_value_policy::reference);                                                                              \
        }                                                                                                                               \
        sparta::StatisticSet::HistogramTnVector histVec= curTn.getStatisticSet()->getHistograms();                                                      \
        for (auto it = histVec.begin(); it != histVec.end(); ++it){                                                                     \
            const std::string &name = (*it)->getName();                                                                                 \
            statisticBind.def_property_readonly(                                                                                        \
                name.c_str(),                                                                                                           \
                [=](UnitClass##Statistics &self) {                                                                                      \
                    sparta::HistogramTreeNode *hist= self.getHistogram(name);                                                                  \
                    return hist;                                                                                                 \
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
    archXplore::python::EmbeddedModule embedded_##UnitClass(&bind_##UnitClass)
