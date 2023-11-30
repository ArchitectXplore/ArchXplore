cmake_minimum_required(VERSION 3.11)
project(pybindSparta LANGUAGES CXX)

# Set up example

add_executable(main Ports_example.cpp ${ArchXplore_Python})

target_include_directories(main PUBLIC .)

target_include_directories(main PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(main PRIVATE ${ArchXplore_LIBS})