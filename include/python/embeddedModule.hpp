#pragma once

#include <vector>
#include <memory>
#include <functional>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include <sparta/sparta.hpp>

namespace archXplore
{

namespace python
{

class embeddedModule
{
public:
    typedef std::function<void(pybind11::module_&)> funcPtrType;

public:
    
    static std::vector<funcPtrType>& getFuncPool() {
        if(m_bindFuncPool == nullptr){
            m_bindFuncPool = std::make_shared<std::vector<funcPtrType>>();
        } 
        return *m_bindFuncPool;
    };

    embeddedModule(funcPtrType func) {
        getFuncPool().push_back(func);
    };
    ~embeddedModule() {};

private:

    static std::shared_ptr<std::vector<funcPtrType>> m_bindFuncPool;

};

} // namespace python

} // namespace archXplore


#define REGISTER_SPARTA_UNIT(UnitClass, ParamClass) \
    using UnitClass##Factory = sparta::ResourceFactory<UnitClass,ParamClass>; \
    class UnitClass##Node : public sparta::ResourceTreeNode { \
    public : \
        UnitClass##Node(TreeNode* parent) : ResourceTreeNode(parent ,#UnitClass"Node", "Resource Tree Node of" #UnitClass "Component", new UnitClass##Factory){}; \
        UnitClass##Node() : ResourceTreeNode(#UnitClass"Node", "Resource Tree Node of" #UnitClass "Component", new UnitClass##Factory){}; \
        ~UnitClass##Node() {};  \
    }; \
    void bind_##UnitClass(pybind11::module_& m) { \
        pybind11::class_<UnitClass>(m, #UnitClass, pybind11::dynamic_attr()); \
        pybind11::class_<ParamClass, sparta::ParameterSet>(m, #ParamClass, pybind11::dynamic_attr()); \
        pybind11::class_<UnitClass##Factory>(m, #UnitClass"Factory", pybind11::dynamic_attr()); \
        pybind11::class_<UnitClass##Node, sparta::TreeNode>(m, #UnitClass"Node", pybind11::dynamic_attr()) \
        .def(pybind11::init<sparta::TreeNode*>()) \
        .def(pybind11::init<>()); \
        ; \
    } \
    archXplore::python::embeddedModule embedded_##UnitClass(&bind_##UnitClass)
