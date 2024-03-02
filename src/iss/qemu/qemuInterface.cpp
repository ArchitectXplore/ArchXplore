#include <csignal>

#include "iss/qemu/qemuInterface.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            const uint32_t max_hart_num = 1024;

            // QEMU Interface Instance variable
            std::mutex g_qemu_if_lock;
            qemuInterface::ptrType g_qemu_if = nullptr;
            // QEMU Thread
            std::thread *m_qemu_thread = nullptr;
            // QEMU operation mutex
            std::shared_mutex m_qemu_sync_lock;
            bool m_qemu_exited = false;
            hartId_t m_simulated_cpu_number = -1;
            // Hart event Queue
            std::vector<hartEventQueue*> m_qemu_event_queue(max_hart_num);

        }
    }
}

extern "C"
{

    QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

    QEMU_PLUGIN_EXPORT
    int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info,
                            int argc, char **argv)
    {
        qemu_plugin_register_vcpu_syscall_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_syscall);
        // qemu_plugin_register_vcpu_syscall_ret_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_syscall_ret);

        qemu_plugin_register_vcpu_init_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_init);
        qemu_plugin_register_vcpu_exit_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_exit);
        qemu_plugin_register_atexit_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_exit, NULL);

        qemu_plugin_register_vcpu_tb_trans_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_tb_trans);

        return 0;
    }
}