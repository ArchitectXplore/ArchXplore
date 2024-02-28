#include <csignal>

#include "iss/qemu/qemuInterface.hpp"

namespace archXplore
{
    namespace iss
    {
        namespace qemu
        {

            // QEMU Interface Instance variable
            std::mutex g_qemu_if_lock;
            qemuInterface::ptrType g_qemu_if = nullptr;
            // QEMU Thread
            std::thread *m_qemu_thread = nullptr;
            // QEMU operation mutex
            std::mutex m_qemu_lock;
            std::shared_mutex m_qemu_sync_lock;
            std::condition_variable m_qemu_cond;
            bool m_qemu_exited = false;
            hartId_t m_simulated_cpu_number = -1;
            // QEMU status
            std::atomic<eventId_t> m_qemu_sync_event_id = 0;
            // Hart event Queue
            hartEventQueue *m_qemu_event_queue[MAX_HART];

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
        qemu_plugin_register_vcpu_init_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_init);
        qemu_plugin_register_vcpu_exit_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_exit);
        qemu_plugin_register_atexit_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_exit, NULL);

        qemu_plugin_register_vcpu_tb_trans_cb(id, &archXplore::iss::qemu::qemuInterface::qemu_vcpu_tb_trans);

        return 0;
    }
}