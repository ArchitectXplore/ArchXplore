

#include <cinttypes>
#include <memory>

#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/ports/PortSet.hpp"
#include "sparta/ports/DataPort.hpp"

#ifdef GPC_DEV
#define PARAMETER_BOOL(name, def, doc) \
    sparta::Parameter<bool> name {#name, def, doc, __this_ps};
#endif

#define ILOG(msg) \
    if(SPARTA_EXPECT_FALSE(info_logger_)) { \
        info_logger_ << msg; \
    }

//
// Basic device parameters
//
class MyDeviceParams : public sparta::ParameterSet
{
public:
    MyDeviceParams(sparta::TreeNode* n) :
        sparta::ParameterSet(n)
    {

        auto a_dumb_true_validator = [](bool & val, const sparta::TreeNode*)->bool {
            // Really dumb validator
            if(val == true) {
                return true;
            }
            return false;
        };
        my_device_param.addDependentValidationCallback(a_dumb_true_validator,
                                                       "My device parameter must be true");
    }

#ifdef GPC_DEV
    PARAMETER_BOOL(my_device_param, true, "An example device parameter")
#else
    PARAMETER(bool, my_device_param, true, "An example device parameter")
#endif

};

//
// Example of a Device in simulation
//
class MyDevice : public sparta::Unit
{
public:
    // Typical and expected constructor signature if this device is
    // build using sparta::ResourceFactory concept
    MyDevice(sparta::TreeNode * parent_node, // The TreeNode this Devive belongs to
             const MyDeviceParams * my_params);

    // Name of this resource. Required by sparta::ResourceFactory.  The
    // code will not compile without it
    static const char * name;

private:
    // A data in port that receives uint32_t
    sparta::DataInPort<uint32_t> a_delay_in_;

    // The callback to receive data from a sender
    void myDataReceiver_(const uint32_t & dat);
};
