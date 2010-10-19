# $Id$

lofar_package(Blob 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa)
lofar_find_package(Blitz)

add_subdirectory(include/Blob)
add_subdirectory(src)
add_subdirectory(test)
