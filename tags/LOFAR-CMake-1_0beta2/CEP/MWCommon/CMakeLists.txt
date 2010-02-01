# $Id$

lofar_package(MWCommon 0.1 DEPENDS Common Blob)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS casa REQUIRED)

add_subdirectory(include/MWCommon)
add_subdirectory(src)
add_subdirectory(test)
