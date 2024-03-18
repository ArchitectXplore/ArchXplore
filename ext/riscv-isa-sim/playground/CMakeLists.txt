  # Find the dynamic loading library

add_executable(playground_main main.cpp)
target_link_libraries(playground_main PRIVATE riscv)
target_link_libraries(playground_main PRIVATE abstract_processor)

find_library(DL_LIBRARY dl)
target_link_libraries(playground_main PRIVATE ${DL_LIBRARY})