cmake_minimum_required(VERSION 3.11)
project(insnTunnelTest LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set up example

add_executable(threadPoolTest threadPool_test.cpp)

target_include_directories(threadPoolTest PUBLIC ${ArchXplore_INCLUDES})

target_include_directories(threadPoolTest PUBLIC .)

target_link_libraries(threadPoolTest PRIVATE pthread)