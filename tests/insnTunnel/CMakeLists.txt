cmake_minimum_required(VERSION 3.11)
project(insnTunnelTest LANGUAGES CXX)

# Set up example

add_executable(insnTunnelTest insnTunnel_test.cpp)

target_include_directories(insnTunnelTest PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(insnTunnelTest PRIVATE ${ArchXplore_LIBS})