# Set the minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.13)

# Define the project name and the language used
project(ARCHXPLORE LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default Build Option -> Debug/Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug/Release)" FORCE)
endif()

# Unset C/CXX Flags
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")

# Set C/CXX Debug Flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Set C/CXX Release Flags
set(CMAKE_C_FLAGS_RELEASE "-g -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

# Add march flags for x86
add_compile_options("-march=native")

set(ArchXplore_BASE ${CMAKE_CURRENT_SOURCE_DIR})

set(ArchXplore_LIBS "")

set(ArchXplore_INCLUDES ${ArchXplore_BASE}/include)

set(ArchXplore_SRCS "")

# Build Marco
include(cmake/buildMacro.cmake)
include(ExternalProject)

# Sparta setup
include(cmake/sparta.cmake)

# QEMU setup
include(cmake/qemu.cmake)

# iceoryx setup
include(cmake/iceoryx.cmake)

# DRAMSim3 setup
include(cmake/DRAMSim3.cmake)

# Add Source Directory
add_subdirectory(src)

# QEMU Plugin
add_library(InstrumentPlugin SHARED src/iss/qemu/InstrumentPlugin.cpp)
target_include_directories(InstrumentPlugin PUBLIC ${ArchXplore_INCLUDES})
target_link_libraries(InstrumentPlugin PRIVATE iceoryx_posh::iceoryx_posh)

# IPC Service
add_executable(IPCService src/iss/IPCService.cpp)
target_include_directories(IPCService PUBLIC ${ArchXplore_INCLUDES})
target_link_libraries(IPCService PRIVATE iceoryx_hoofs::iceoryx_hoofs iceoryx_posh::iceoryx_posh_roudi)

# Main
add_executable(ArchXplore src/python/ArchXplorePy.cpp ${ArchXplore_SRCS})
target_include_directories(ArchXplore PUBLIC ${ArchXplore_INCLUDES})
target_link_libraries(ArchXplore PRIVATE ${ArchXplore_LIBS})
set_target_properties(ArchXplore PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Generate Python Stubs
add_custom_command(TARGET ArchXplore POST_BUILD
    COMMENT "[INFO] Install python stub into ${CMAKE_BINARY_DIR}/stubs !" 
    COMMAND ./ArchXplore ${ArchXplore_BASE}/python/util/stubgen.py -o ${CMAKE_BINARY_DIR}/stubs archXplore 
)

# Add Tests
add_subdirectory(tests EXCLUDE_FROM_ALL)
