
#include <cinttypes>
#include <memory>

#include "sparta/simulation/ParameterSet.hpp"
 

#define PARAMETER_BOOL(name, def, doc) \
    sparta::Parameter<bool> name {#name, def, doc, __this_ps};

#define PARAMETER_U32(name, def, doc) \
    sparta::Parameter<uint32_t> name {#name, def, doc, __this_ps};

#define PARAMETER_DOUBLE(name, def, doc) \
    sparta::Parameter<double> name {#name, def, doc, __this_ps};

#define PARAMETER_STR(name, def, doc) \
    sparta::Parameter<std::string> name {#name, def, doc, __this_ps};