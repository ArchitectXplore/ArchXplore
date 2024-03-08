cmake_minimum_required(VERSION 3.11)
project(SpartaInPythonTest LANGUAGES CXX)

# Set up example

add_executable(SpartaSchedulerPerfTest SpartaSchedulerPerf_test.cpp)

target_include_directories(SpartaSchedulerPerfTest PUBLIC .)

target_include_directories(SpartaSchedulerPerfTest PUBLIC ${SPARTA_INCLUDE_DIRS})

target_link_libraries(SpartaSchedulerPerfTest PRIVATE ${Sparta_LIBS})
