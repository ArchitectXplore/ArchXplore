#include "python/embeddedModule.hpp"
#include "python/spartaModules.hpp"
#include "python/commonModules.hpp"
namespace archXplore
{

    namespace python
    {
        std::shared_ptr<std::vector<embeddedModule::funcPtrType>> embeddedModule::m_bindFuncPool = nullptr;

        PYBIND11_EMBEDDED_MODULE(archXplore, m)
        {

            sparta::bindSpartaModules(m);

            bindCommonModules(m);

            auto &funcPool = embeddedModule::getFuncPool();
            // Execute pybind11 functions
            for (auto it = funcPool.begin(); it != funcPool.end(); ++it)
            {
                it->operator()(m);
            }
        }

    } // namespace python

} // namespace archXplore
