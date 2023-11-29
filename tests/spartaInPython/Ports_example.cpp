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
