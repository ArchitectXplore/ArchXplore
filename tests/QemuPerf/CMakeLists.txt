cmake_minimum_required(VERSION 3.11)
project(QemuPerfTest LANGUAGES CXX)

# Set up example

add_executable(QemuPerfTest qemuPerf_test.cpp)

target_include_directories(QemuPerfTest PUBLIC .)

target_include_directories(QemuPerfTest PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(QemuPerfTest PRIVATE ${ArchXplore_LIBS} InstrumentPlugin)
