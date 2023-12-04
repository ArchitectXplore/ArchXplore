macro (add_sources)
    foreach (_src ${ARGN})
        list (APPEND ArchXplore_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${_src}")
    endforeach()
    # propagate SRCS to parent directory
    set (ArchXplore_SRCS ${ArchXplore_SRCS} PARENT_SCOPE)
endmacro()