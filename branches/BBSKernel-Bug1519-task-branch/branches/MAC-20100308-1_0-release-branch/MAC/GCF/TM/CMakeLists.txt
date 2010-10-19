# $Id$

lofar_package(GCFTM 6.1 DEPENDS Common MACIO)

include(LofarFindPackage)
lofar_find_package(Boost)

add_subdirectory(include/GCF/TM)
add_subdirectory(src)
add_subdirectory(test)
