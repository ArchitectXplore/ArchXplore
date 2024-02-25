cmake_minimum_required(VERSION 3.11)
project(qemuPerfTest LANGUAGES CXX)

# Set up example

add_executable(qemuPerfTest spartaSchedulerPerf_test.cpp)

target_include_directories(qemuPerfTest PUBLIC .)

target_include_directories(qemuPerfTest PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(qemuPerfTest PRIVATE ${ArchXplore_LIBS} qemuInterface_plugin)
