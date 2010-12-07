# $Id$

lofar_package(ParmDB 1.0 DEPENDS Common Blob MWCommon)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa tables REQUIRED)
lofar_find_package(Boost REQUIRED)

add_subdirectory(include/ParmDB)
add_subdirectory(src)
add_subdirectory(test)
