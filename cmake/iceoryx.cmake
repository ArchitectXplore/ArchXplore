# iceoryx as an External Project
# ===========================
set(ICEORYX_DIR "${ArchXplore_BASE}/ext/iceoryx/iceoryx_meta")

add_subdirectory(${ICEORYX_DIR} iceoryx)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

list(APPEND ArchXplore_LIBS iceoryx_posh::iceoryx_posh)
