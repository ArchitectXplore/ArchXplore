# DRAMSim3 as an External Project
# ===========================
set(DRAMSim3_DIR "${ArchXplore_BASE}/ext/DRAMsim3")

add_subdirectory(${DRAMSim3_DIR} DRAMSim3)

list(APPEND ArchXplore_LIBS dramsim3)
