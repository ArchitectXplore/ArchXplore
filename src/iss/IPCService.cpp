
#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iox/logging.hpp"

#include "iss/IPCConfig.hpp"

int main(int argc, char *argv[])
{
    using iox::roudi::IceOryxRouDiApp;

    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    auto cmdLineArgs = cmdLineParser.parse(argc, argv);
    if (cmdLineArgs.has_error())
    {
        IOX_LOG(FATAL, "Unable to parse command line arguments!");
        return EXIT_FAILURE;
    }

    if (!cmdLineArgs.value().run)
    {
        return EXIT_SUCCESS;
    }

    iox::IceoryxConfig config;
    // config.setDefaults(); can be used if you want to use the default config only.
    static_cast<iox::config::RouDiConfig &>(config) = cmdLineArgs.value().roudiConfig;

    /// @brief Create Mempool Config
    iox::mepoo::MePooConfig mepooConfig;

    /// @details Format: addMemPool({Chunksize(bytes), Amount of Chunks})
    mepooConfig.addMemPool({archXplore::iss::MESSAGE_CHUNK_SIZE, archXplore::iss::MAX_HARTS * archXplore::iss::MESSAGE_BUFFER_SIZE * 3}); // bytes

    /// We want to use the Shared Memory Segment for the current user
    auto currentGroup = iox::PosixGroup::getGroupOfCurrentProcess();

    /// Create an Entry for a new Shared Memory Segment from the MempoolConfig and add it to the IceoryxConfig
    config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mepooConfig});

    /// configure the chunk count for the introspection; each introspection topic gets this number of chunks
    config.introspectionChunkCount = 10;

    /// configure the chunk count for the service discovery
    config.discoveryChunkCount = 10;

    IceOryxRouDiApp roudi(config);

    return roudi.run();
}
