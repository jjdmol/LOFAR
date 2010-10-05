# $Id$

lofar_package(MS 0.1 DEPENDS Common Blob MWCommon)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures ms tables REQUIRED)

add_subdirectory(include/MS)
add_subdirectory(src)
add_subdirectory(test)
