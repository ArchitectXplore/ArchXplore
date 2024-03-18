file(GLOB RISCV_SRC ./*.cc)
file(GLOB INSTR_SRC ../spikebuild/*.cc)
add_library(riscv
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/cachesim.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/cfg.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/check-opcode-overlap.t.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/csrs.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/clint.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/devices.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/execute.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/extension.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/extensions.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/jtag_dtm.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/mmu.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/processor.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/remote_bitbang.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/rocc.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/rom.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/socketif.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/triggers.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/riscv/vector_unit.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/disasm/isa_parser.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/disasm/disasm.cc
    /work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim/disasm/regnames.cc
    ${INSTR_SRC}
    )

target_link_libraries(riscv PRIVATE softfloat)
# target_include_directories(riscv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(riscv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(riscv PUBLIC ${CMAKE_SOURCE_DIR}/build)
target_include_directories(riscv PUBLIC ${CMAKE_SOURCE_DIR}/softfloat)
# target_include_directories(riscv PUBLIC ${CMAKE_SOURCE_DIR}/abstract_processor)
target_include_directories(riscv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/insns)

find_library(DL_LIBRARY dl)
target_link_libraries(riscv PRIVATE ${DL_LIBRARY})