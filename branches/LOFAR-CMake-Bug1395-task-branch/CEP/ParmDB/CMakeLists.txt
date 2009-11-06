# $Id$

lofar_package(ParmDB 1.0 DEPENDS Common Blob MWCommon)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa tables REQUIRED)

add_subdirectory(include/ParmDB)
add_subdirectory(src)
add_subdirectory(sql)
add_subdirectory(test)
