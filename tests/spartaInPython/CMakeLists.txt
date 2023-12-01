cmake_minimum_required(VERSION 3.11)
project(spartaInPythonTest LANGUAGES CXX)

# Set up example

add_executable(spartaInPythonTest Ports_example.cpp ${ArchXplore_SRCS})

target_include_directories(spartaInPythonTest PUBLIC .)

target_include_directories(spartaInPythonTest PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(spartaInPythonTest PRIVATE ${ArchXplore_LIBS})

add_custom_command(TARGET spartaInPythonTest POST_BUILD
    COMMENT "[INFO] Install python stub into ${CMAKE_BINARY_DIR}/stubs !" 
    COMMAND ./spartaInPythonTest ${ArchXplore_BASE}/python/util/stubgen.py -o ${CMAKE_BINARY_DIR}/stubs archXplore 
)