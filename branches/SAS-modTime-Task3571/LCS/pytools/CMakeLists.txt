# $Id$

lofar_package(pytools 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python REQUIRED)

add_subdirectory(include/pytools)
add_subdirectory(src)
add_subdirectory(test)
