# $Id$

lofar_package(BBSKernel 1.0 DEPENDS Blob Common ParmDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS ms REQUIRED)

add_definitions(-Wconversion)

add_subdirectory(include/BBSKernel)
add_subdirectory(src)
add_subdirectory(test)
