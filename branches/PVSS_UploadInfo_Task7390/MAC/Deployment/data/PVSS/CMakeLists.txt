# $Id$

lofar_package(PVSS_Datapoints 1.0 DEPENDS Common OTDB)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures REQUIRED)
lofar_find_package(Boost REQUIRED date_time)

add_subdirectory(bin)
add_subdirectory(src)
add_subdirectory(data)
