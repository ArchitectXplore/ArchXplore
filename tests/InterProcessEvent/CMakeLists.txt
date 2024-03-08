cmake_minimum_required(VERSION 3.11)
project(InterProcessEventTest LANGUAGES CXX)

# Set the C++ standard you wish to use (you could use C++11, C++14, C++17, etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set up example

add_executable(Subscriber Subscriber.cpp)

target_include_directories(Subscriber PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(Subscriber PRIVATE ${ArchXplore_LIBS})


add_executable(Publisher Publisher.cpp)

target_include_directories(Publisher PUBLIC ${ArchXplore_INCLUDES})

target_link_libraries(Publisher PRIVATE ${ArchXplore_LIBS})
