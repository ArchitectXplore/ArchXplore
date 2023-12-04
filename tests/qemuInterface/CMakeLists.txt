cmake_minimum_required(VERSION 3.11)
project(qemuInterfaceTest LANGUAGES CXX)

# System Mode Tests
add_executable(qemuInterfaceTestLinux)

target_include_directories(qemuInterfaceTestLinux PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(qemuInterfaceTestLinux PRIVATE ${ArchXplore_LIBS})
