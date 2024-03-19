add_subdirectory(iss)
add_subdirectory(python)
add_subdirectory(system)
add_subdirectory(cpu)
add_subdirectory(uncore)

add_sources(ClockedObject.cpp)

set(ArchXplore_SRCS ${ArchXplore_SRCS} PARENT_SCOPE)