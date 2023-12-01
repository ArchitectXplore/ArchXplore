
# PYBIND11 as an External Project
# ===========================
set(PYBIND11_DIR "${ArchXplore_BASE}/ext/pybind11")

# Set up python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

find_package(pybind11 REQUIRED)

list(APPEND ArchXplore_LIBS pybind11::headers Python3::Python)

list(APPEND ArchXplore_INCLUDES ${Python3_INCLUDE_DIRS})