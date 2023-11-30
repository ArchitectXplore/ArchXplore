# Set the minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.10)

# Define the project name and the language used
project(ARCHXPLORE LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build Option -> Debug/Release
option(DEBUG "Debug option" OFF)
if(DEBUG) 
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

set(ArchXplore_BASE ${CMAKE_CURRENT_SOURCE_DIR})

set(ArchXplore_LIBS "")

set(ArchXplore_INCLUDES ${ArchXplore_BASE}/include)

set(ArchXplore_SRCS "")

# Build Marco
include(cmake/buildMacro.cmake)

# Pybind11 setup
include(cmake/pybind11.cmake)

# Sparta setup
include(cmake/sparta.cmake)

# Add Source Directory
file(GLOB ArchXplore_Python "src/python/*.cpp")

list(APPEND ArchXplore_SRCS
    ${ArchXplore_Python}
)

# Add Tests
add_subdirectory(tests EXCLUDE_FROM_ALL)

