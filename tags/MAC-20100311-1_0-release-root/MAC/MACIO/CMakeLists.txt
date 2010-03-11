# $Id$

lofar_package(MACIO 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Boost)

include_directories(${CMAKE_BINARY_DIR}/include/MAC)

add_subdirectory(autogen)
add_subdirectory(include/MACIO)
add_subdirectory(src)
add_subdirectory(test)
