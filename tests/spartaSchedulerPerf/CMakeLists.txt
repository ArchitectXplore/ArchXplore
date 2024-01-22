cmake_minimum_required(VERSION 3.11)
project(spartaInPythonTest LANGUAGES CXX)

# Set up example

add_executable(spartaSchedulerPerfTest spartaSchedulerPerf_test.cpp)

target_include_directories(spartaSchedulerPerfTest PUBLIC .)

target_include_directories(spartaSchedulerPerfTest PUBLIC ${SPARTA_INCLUDE_DIRS})

target_link_libraries(spartaSchedulerPerfTest PRIVATE ${Sparta_LIBS})
