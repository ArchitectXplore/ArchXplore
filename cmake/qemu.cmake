# QEMU as an External Project
# ===========================
set(QEMU_DIR "${ArchXplore_BASE}/ext/qemu")

if(CMAKE_BUILD_TYPE STREQUAL Debug) 
    set(QEMU_DEBUG_FLAG --enable-debug)
else()
    set(QEMU_DEBUG_FLAG )
endif()

ExternalProject_Add(qemu
    SOURCE_DIR        ${QEMU_DIR}                     # Source directory for QEMU
    BINARY_DIR        ${CMAKE_BINARY_DIR}/qemu        # Build directory for QEMU
    CONFIGURE_COMMAND CC=${CC} CFLAGS=${CMAKE_C_FLAGS} ${QEMU_DIR}/configure --target-list=riscv64-linux-user --enable-plugins ${QEMU_DEBUG_FLAG}   # Configure command (e.g., ./configure)
    BUILD_COMMAND     make -j${nproc}                 # Build command (e.g., make)
    INSTALL_COMMAND   ""                              # Install command (e.g., make install)
    TEST_COMMAND      ""                              # Disable test command
)

ExternalProject_Get_Property(qemu BINARY_DIR)