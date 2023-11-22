cmake_minimum_required(VERSION 3.11)
project(pybindSparta LANGUAGES CXX)
 

# Set up Sparta
if(IS_DIRECTORY ${SPARTA_SEARCH_DIR})
  message (STATUS "Using '${SPARTA_SEARCH_DIR}' for sparta install")
elseif(DEFINED ENV{CONDA_PREFIX})
  message (STATUS "Looking for SPARTA in the conda environment: '$ENV{CONDA_PREFIX}'")
  set(SPARTA_SEARCH_DIR $ENV{CONDA_PREFIX})
  message (STATUS "If needed, please provide the location where sparta is installed: -DSPARTA_SEARCH_DIR=<directory>")
else()
  message (STATUS "If needed, please provide the location where sparta is installed: -DSPARTA_SEARCH_DIR=<directory>")
endif()
set(CMAKE_MODULE_PATH "${SPARTA_SEARCH_DIR}/lib/cmake/sparta" ${CMAKE_MODULE_PATH})

find_package(Sparta REQUIRED)

if (NOT SPARTA_FOUND)
  message (FATAL_ERROR "Sparta was not found in ${SPARTA_SEARCH_DIR}")
else()
  message (STATUS "Sparta was found in ${SPARTA_SEARCH_DIR}")
endif()

include_directories(${SPARTA_INCLUDE_DIRS})
include(${SPARTA_SEARCH_DIR}/lib/cmake/sparta/sparta-config.cmake)

#yaml-cpp
find_package(yaml-cpp REQUIRED)
#HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX)
# if (NOT YAML-CPP_FOUND)
#   message (FATAL_ERROR "YAML-CPP was not found ")
# else()
#   message (STATUS "YAML-CPP was found ")
# endif()

# Set up pybind
add_subdirectory(../../ext/pybind11 ./build)

# Set up example
pybind11_add_module(example example.cpp)
pybind11_add_module(pysparta pybindSparta.cpp)
target_link_libraries(pysparta PRIVATE ${Sparta_LIBS})

# add_custom_target(python
#     COMMAND make -j 4
#     COMMAND python3 main.py
#     COMMENT "Running make -j 4 and python3 main.py"
# )