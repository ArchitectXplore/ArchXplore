# QEMU as an External Project
# ===========================
set(QEMU_DIR "${ArchXplore_BASE}/ext/qemu")

ExternalProject_Add(qemu
    SOURCE_DIR        ${QEMU_DIR}                     # Source directory for QEMU
    BINARY_DIR        ${CMAKE_BINARY_DIR}/qemu        # Build directory for QEMU
    CONFIGURE_COMMAND CC=${CC} CXX=${CXX} ${QEMU_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX} --target-list=riscv64-linux-user,riscv64-softmmu --enable-plugins    # Configure command (e.g., ./configure)
    BUILD_COMMAND     make -j${nproc}                 # Build command (e.g., make)
    INSTALL_COMMAND   ""                              # Install command (e.g., make install)
    TEST_COMMAND      ""                              # Disable test command
)


# list(APPEND ArchXplore_LIBS ${Sparta_LIBS})

# list(APPEND ArchXplore_INCLUDES ${SPARTA_INCLUDE_DIRS})

# link_directories(${PROJECT_BINARY_DIR})