cmake_minimum_required(VERSION 3.11)
project(pybindSparta LANGUAGES CXX)
 
# Set up python
find_package(Python3 COMPONENTS Interpreter Development)

# Set up sparta
find_package(Sparta REQUIRED)

# Set up pybind
add_subdirectory(../../ext/pybind11 ./pybind11)

# Set up example

file(GLOB SRC_LIST "../../src/python/*.cpp")

add_executable(main ${SRC_LIST} Ports_example.cpp)

target_include_directories(main PUBLIC ../../include)

target_include_directories(main PUBLIC .)

target_include_directories(main PUBLIC ${SPARTA_INCLUDE_DIRS})

target_link_libraries(main PRIVATE pybind11::headers)

target_link_libraries(main PRIVATE Python3::Python)

target_link_libraries(main PRIVATE ${Sparta_LIBS})

# install(CODE "execute_process(COMMAND ./main /home/lzhang/miniconda3/envs/sparta/bin/pybind11-stubgen archXplore")