cmake_minimum_required(VERSION 3.11)
project(SpartaInPythonTest LANGUAGES CXX)

# Set up example

add_executable(SpartaInPythonTest ${ArchXplore_BASE}/src/python/ArchXplorePy.cpp Ports_example.cpp ${ArchXplore_SRCS})

target_include_directories(SpartaInPythonTest PUBLIC .)

target_include_directories(SpartaInPythonTest PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(SpartaInPythonTest PRIVATE ${ArchXplore_LIBS})

add_custom_command(TARGET SpartaInPythonTest POST_BUILD
    COMMENT "[INFO] Install python stub into ${CMAKE_BINARY_DIR}/stubs !" 
    COMMAND ./SpartaInPythonTest ${ArchXplore_BASE}/python/util/stubgen.py -o ${CMAKE_BINARY_DIR}/stubs archXplore 
)