# $Id$

lofar_package(DPPP 1.0 DEPENDS Common MS ParmDB StationResponse)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(AOFlagger)

add_subdirectory(include/DPPP)
add_subdirectory(src)
add_subdirectory(share)
add_subdirectory(test)
