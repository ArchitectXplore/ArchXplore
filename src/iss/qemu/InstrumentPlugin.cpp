#include "iss/qemu/instrumentation/InstrumentPlugin.hpp"

namespace archXplore
{

    namespace iss
    {

        namespace qemu
        {

            // Application name
            std::string InstrumentPlugin::m_app_name;
            // Process information
            ProcessID_t InstrumentPlugin::m_pid;
            // Boot hart ID
            HartID_t InstrumentPlugin::m_boot_hart;
            // Maximum number of harts
            HartID_t InstrumentPlugin::m_max_harts;

            // Last executed instructions for each VCPU
            std::unordered_map<HartID_t, cpu::StaticInst_t> InstrumentPlugin::m_last_inst_map;
            // Instruction cache
            InstrumentPlugin::InstCache_t InstrumentPlugin::m_inst_cache;
            // Mutex for instruction cache
            std::shared_mutex InstrumentPlugin::m_inst_cache_mutex;
            // Event publisher
            std::unordered_map<HartID_t, std::unique_ptr<EventPublisher>> InstrumentPlugin::m_event_publishers;

        } // namespace qemu

    } // namespace iss

} // namespace archXplore

// Plugin entry point
extern "C"
{
    QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

    void print_usage()
    {
        std::string usage = "\nUsage: -plugin=<plugin name>,AppName=<app name>,"
                            "ProcessID=<process ID>,BootHart=<boot hart ID>,"
                            "MaxHarts=< maximum number of harts >\n";
        std::cerr << usage << std::endl;
        std::exit(1);
    }

    QEMU_PLUGIN_EXPORT
    int qemu_plugin_install(qemu_plugin_id_t id, const qemu_info_t *info,
                            int argc, char **argv)
    {

        std::string arg, key, value;
        if (argc < 4)
        {
            print_usage();
        }
        else
        {
            // Parse command line arguments
            for (int i = 0; i < argc; i++)
            {
                arg = argv[i];
                key = arg.substr(0, arg.find("="));
                value = arg.substr(arg.find("=") + 1);
                if (key == "AppName")
                {
                    archXplore::iss::qemu::InstrumentPlugin::m_app_name = value;
                }
                else if (key == "ProcessID")
                {
                    archXplore::iss::qemu::InstrumentPlugin::m_pid = std::stoi(value);
                }
                else if (key == "BootHart")
                {
                    archXplore::iss::qemu::InstrumentPlugin::m_boot_hart = std::stoi(value);
                }
                else if (key == "MaxHarts")
                {
                    archXplore::iss::qemu::InstrumentPlugin::m_max_harts = std::stoi(value);
                }
                else
                {
                    print_usage();
                }
            }
        }

        // Register the instrumentation plugin
        qemu_plugin_register_vcpu_tb_trans_cb(id, archXplore::iss::qemu::InstrumentPlugin::translateBasicBlock);

        qemu_plugin_register_vcpu_syscall_cb(id, archXplore::iss::qemu::InstrumentPlugin::syscall);

        qemu_plugin_register_vcpu_init_cb(id, archXplore::iss::qemu::InstrumentPlugin::threadInitialize);

        qemu_plugin_register_vcpu_exit_cb(id, archXplore::iss::qemu::InstrumentPlugin::threadExit);

        qemu_plugin_register_atexit_cb(id, archXplore::iss::qemu::InstrumentPlugin::qemuAtExit, NULL);

        return 0;
    }
}
