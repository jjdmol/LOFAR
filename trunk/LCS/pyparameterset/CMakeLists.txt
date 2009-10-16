# $Id$

lofar_package(pyparameterset 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python REQUIRED)
lofar_find_package(Pyrap REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
