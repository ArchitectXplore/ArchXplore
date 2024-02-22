# Set the minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.13)

# Define the project name and the language used
project(ARCHXPLORE LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set C/C++ compiler you with to use
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

# Reset CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "-g")

# Default Build Option -> Debug/Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug/Release)" FORCE)
endif()

# Build QEMU User Mode 
add_definitions(-DCONFIG_USER_ONLY)

set(ArchXplore_BASE ${CMAKE_CURRENT_SOURCE_DIR})

set(ArchXplore_LIBS "")

set(ArchXplore_INCLUDES ${ArchXplore_BASE}/include)

set(ArchXplore_SRCS "")

# Build Marco
include(cmake/buildMacro.cmake)
include(ExternalProject)

# Pybind11 setup
include(cmake/pybind11.cmake)

# Sparta setup
include(cmake/sparta.cmake)

# QEMU setup
include(cmake/qemu.cmake)

# Add Source Directory
add_subdirectory(src)

# QEMU Plugin
add_library(qemuInterface_plugin SHARED src/iss/qemu/qemuInterface.cpp)
target_include_directories(qemuInterface_plugin PUBLIC ${ArchXplore_INCLUDES})
target_link_libraries(qemuInterface_plugin PRIVATE ${ArchXplore_LIBS})


# Main
add_executable(main src/python/archXplorePy.cpp ${ArchXplore_SRCS})
target_include_directories(main PUBLIC ${ArchXplore_INCLUDES})
target_link_libraries(main PRIVATE ${ArchXplore_LIBS} qemuInterface_plugin)
set_target_properties(main PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_dependencies(main qemu)


# Generate Python Stubs
add_custom_command(TARGET main POST_BUILD
    COMMENT "[INFO] Install python stub into ${CMAKE_BINARY_DIR}/stubs !" 
    COMMAND ./main ${ArchXplore_BASE}/python/util/stubgen.py -o ${CMAKE_BINARY_DIR}/stubs archXplore 
)

# Add Tests
add_subdirectory(tests EXCLUDE_FROM_ALL)