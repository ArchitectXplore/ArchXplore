#include <iss/qemu/qemuInterface.h>

namespace archXplore{
namespace iss{
namespace qemu{

std::shared_ptr<qemuInterface> g_qemuInterface_instance = nullptr;
std::mutex g_qemuInterface_lock;


} // namespace qemu
} // namespace iss
} // namespace ArchXplore
