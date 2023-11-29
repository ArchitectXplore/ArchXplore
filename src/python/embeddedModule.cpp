#include "python/embeddedModule.hpp"

namespace archXplore
{

namespace python
{

std::shared_ptr<std::vector<embeddedModule::funcPtrType>> embeddedModule::m_bindFuncPool = nullptr;


PYBIND11_EMBEDDED_MODULE(archXplore, m) {

    auto& funcPoor = embeddedModule::getFuncPool();
    // Execute pybind11 functions 
    for(auto it = funcPoor.begin(); it != funcPoor.end(); ++it){
        it->operator()(m);
    }

}


} // namespace python

    
} // namespace archXplore
