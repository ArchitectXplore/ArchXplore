# SPARTA as an External Project
# ===========================
set(SPARTA_DIR "${ArchXplore_BASE}/ext/map/sparta")
set(SPARTA_INCLUDE_DIRS "${SPARTA_DIR}/include")

add_subdirectory(${SPARTA_DIR})

include(${SPARTA_DIR}/cmake/sparta-config.cmake)

list(APPEND ArchXplore_LIBS ${Sparta_LIBS})

list(APPEND ArchXplore_INCLUDES ${SPARTA_INCLUDE_DIRS})

link_directories(${PROJECT_BINARY_DIR})