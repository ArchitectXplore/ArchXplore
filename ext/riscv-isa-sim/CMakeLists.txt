cmake_minimum_required (VERSION 3.13)

PROJECT(Spike)
SET(CMAKE_CXX_STANDARD 17)

# add_definitions(-DARCHXPLORE_WBSPLITE)
add_definitions(-DSPIKE_ABSTRACT_PROCESSOR)
set(CMAKE_BUILD_TYPE "Debug")
# MESSAGE(STATUS "THIS IS BIN DIR" ${CMAKE_BINARY_DIR})
# MESSAGE(STATUS "THIS IS SRC DIR" ${CMAKE_SOURCE_DIR})

# execute_process(
#     COMMAND ../configure --prefix=$RISCV --without-boost --without-boost-asio --without-boost-regex
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
# )

# execute_process(
#     COMMAND make -j
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build
# )

# include_directories(${CMAKE_SOURCE_DIR}/fesvr)
include_directories(${CMAKE_SOURCE_DIR}/riscv)
include_directories(${CMAKE_SOURCE_DIR}/spikebuild)
include_directories(${CMAKE_SOURCE_DIR}/playground)
include_directories(${CMAKE_SOURCE_DIR}/abstract_processor)



add_subdirectory(softfloat)
add_subdirectory(riscv)
add_subdirectory(playground)
add_subdirectory(abstract_processor)
# add_subdirectory(fesvr)

