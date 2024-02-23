# SPARTA as an External Project
# ===========================
set(GEM5_DIR "${ArchXplore_BASE}/ext/gem5")
set(GEM5_INCLUDE_DIRS "${GEM5_DIR}/src")

ExternalProject_Add(gem5
    SOURCE_DIR        ${GEM5_DIR}                     # Source directory for QEMU
    BINARY_DIR        ${CMAKE_BINARY_DIR}/gem5        # Build directory for QEMU
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""                              # Install command (e.g., make install)
    TEST_COMMAND      ""                              # Disable test command
)
# BUILD_COMMAND     scons build/RISCV/libgem5_opt.so -j ${nproc} --without-tcmalloc --duplicate-sources # Build command (e.g., make)

ExternalProject_Get_Property(qemu BINARY_DIR)

link_directories(${ArchXplore_BASE}/ext/gem5/build/RISCV)

list(APPEND ArchXplore_LIBS gem5_opt)

list(APPEND ArchXplore_INCLUDES ${GEM5_INCLUDE_DIRS})
