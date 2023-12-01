
target_sources(src_lib PRIVATE qemuInterface.cpp)

add_library(qemuInterface_plugin SHARED qemuInterface_plugin.cpp)