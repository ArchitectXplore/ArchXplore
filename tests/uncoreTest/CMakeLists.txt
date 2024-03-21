cmake_minimum_required(VERSION 3.11)
project(ThreadPoolTest LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set up example

add_executable(dramsim3_test 
    ${ArchXplore_BASE}/src/python/ArchXplorePy.cpp
    ${ArchXplore_SRCS} 
    random_tester.cpp
    fake_memory.cpp
)

target_include_directories(dramsim3_test PUBLIC ${ArchXplore_INCLUDES})

target_include_directories(dramsim3_test PUBLIC .)

target_link_libraries(dramsim3_test PRIVATE dramsim3 inih ${ArchXplore_LIBS})

add_custom_command(TARGET dramsim3_test POST_BUILD
    COMMENT "[INFO] Install python stub into ${CMAKE_BINARY_DIR}/stubs !" 
    COMMAND ./dramsim3_test ${ArchXplore_BASE}/python/util/stubgen.py -o ${CMAKE_BINARY_DIR}/stubs archXplore 
)