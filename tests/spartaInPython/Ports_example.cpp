#include "Ports_example.hpp"
#include "python/embeddedModule.hpp"


// Defined name
const char * MyDevice::name = "my_device";

////////////////////////////////////////////////////////////////////////////////
// Implementation

// Construction
MyDevice::MyDevice(sparta::TreeNode * my_node,
                   const MyDeviceParams * my_params) :
    sparta::Unit(my_node, name),
    a_delay_in_(&unit_port_set_, "a_delay_in", 1) // Receive data one cycle later
{
    // Tell SPARTA to ignore this parameter
    my_params->my_device_param.ignore();

    // Register the callback
    a_delay_in_.
        registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(MyDevice, myDataReceiver_, uint32_t));
}

// This function will be called when a sender with a DataOutPort
// sends data on its out port.  An example would look like:
//
//     a_delay_out.send(1234);
//
void MyDevice::myDataReceiver_(const uint32_t & dat)
{
    ILOG("I got data: " << dat);
}


REGISTER_SPARTA_UNIT(MyDevice, MyDeviceParams);

// using MyDeviceFactory = sparta::ResourceFactory<MyDevice,MyDeviceParams>;
// class MyDeviceNode : public sparta::ResourceTreeNode { 
// public : 
//     MyDeviceNode(TreeNode* parent) 
//         : ResourceTreeNode(parent ,"MyDevice""Node", "Resource Tree Node of" "MyDevice" "Component", new MyDeviceFactory){}; 
//     MyDeviceNode() 
//         : ResourceTreeNode("MyDevice""Node", "Resource Tree Node of" "MyDevice" "Component", new MyDeviceFactory){};
//     ~MyDeviceNode() {};
// }; 

// class MyDevicePorts{ 
// public : 
//     MyDevicePorts(sparta::PortSet * ptr) : m_ports(ptr) {};
//     ~MyDevicePorts() {};
//     sparta::Port* getPort(const std::string& name) {
//         return m_ports->getPort(name);
//     };
// private : 
//     sparta::PortSet * m_ports;
// }; 

// void bind_MyDevice(pybind11::module_& m) { 
//     auto unitBind = pybind11::class_<MyDevice>(m, "MyDevice", pybind11::dynamic_attr()); 
//     auto paramBind = pybind11::class_<MyDeviceParams>(m, "MyDeviceParams", pybind11::dynamic_attr()); 
//     auto portBind = pybind11::class_<MyDevicePorts>(m, "MyDevicePorts", pybind11::dynamic_attr()); 
//     auto factoryBind = pybind11::class_<MyDeviceFactory>(m, "MyDevice""Factory", pybind11::dynamic_attr()); 
//     auto componentBind = pybind11::class_<MyDeviceNode, sparta::ResourceTreeNode>(m, "MyDevice""Node", pybind11::dynamic_attr()) 
//         .def(pybind11::init<sparta::TreeNode*>()) 
//         .def(pybind11::init<>());

//     // Component Dynamic Attribute 
//     componentBind.def_property_readonly("Params",
//         [](MyDeviceNode& self) {
//             return static_cast<MyDeviceParams*>(self.getParameterSet());
//         }, pybind11::return_value_policy::reference
//     );

//     componentBind.def_property_readonly("Ports",
//         [](MyDeviceNode& self) {
//             MyDevicePorts* myPorts = new MyDevicePorts(self.getResourceAs<sparta::Unit>()->getPortSet());
//             return myPorts;
//         }, pybind11::return_value_policy::reference
//     );

//     // Pseudo Finalize
//     sparta::RootTreeNode rtn;
//     MyDeviceNode curTn(&rtn);
//     sparta::Scheduler scheduler;
//     sparta::Clock clk("clk", &scheduler);
//     rtn.setClock(&clk);
//     rtn.enterConfiguring();
//     rtn.enterFinalized();

//     // Parameter Dynamic Attribute 
//     sparta::TreeNode::ChildrenVector paramVec = curTn.getParameterSet()->getChildren();
//     for(auto it = paramVec.begin(); it != paramVec.end(); ++it) {
//         const std::string& name = (*it)->getName();
//         paramBind.def_property_readonly(name.c_str(),
//             [=](MyDeviceParams& self) {
//                 return self.getParameter(name);
//             }, pybind11::return_value_policy::reference
//         );
//     }

//     // Port Dynamic Attribute 
//     sparta::TreeNode::ChildrenVector portVec = curTn.getResourceAs<sparta::Unit>()->getPortSet()->getChildren();
//     for(auto it = portVec.begin(); it != portVec.end(); it++) {
//         const std::string& name = (*it)->getName().c_str();
//         portBind.def_property_readonly(name.c_str(), 
//             [=](MyDevicePorts& self) {
//                 return self.getPort(name);
//             }, pybind11::return_value_policy::reference
//         );
//     }

//     rtn.enterTeardown();
// } 
    
// archXplore::python::embeddedModule embedded_MyDevice(&bind_MyDevice);