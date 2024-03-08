#include "python/EmbeddedModule.hpp"
#include "python/SpartaModules.hpp"
#include "python/CommonModules.hpp"
namespace archXplore
{

    namespace python
    {
        std::shared_ptr<std::vector<EmbeddedModule::FuncPtr_t>> EmbeddedModule::m_bindFuncPool = nullptr;

        PYBIND11_EMBEDDED_MODULE(archXplore, m)
        {

            // Bind sparta modules
            bindSpartaModules(m);
            // Bind common modules
            bindCommonModules(m);
            // Bind custom modules here
            auto &funcPool = EmbeddedModule::getFuncPool();
            // Execute pybind11 functions
            for (auto it = funcPool.begin(); it != funcPool.end(); ++it)
            {
                it->operator()(m);
            }
        }

    } // namespace python

} // namespace archXplore
