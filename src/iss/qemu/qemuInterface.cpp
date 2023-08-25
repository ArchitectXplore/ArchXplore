#include <iss/qemu/qemuEmulator.h>

namespace ArchXplore
{
namespace iss
{
namespace qemu
{

extern std::shared_ptr<qemuInterface> g_qemuInterface_instance;
extern std::mutex g_qemuInterface_lock;


} // namespace qemu
} // namespace iss
} // namespace ArchXplore
