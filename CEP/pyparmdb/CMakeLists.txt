# $Id$

lofar_package(pyparmdb 1.0 DEPENDS Common ParmDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python REQUIRED)
lofar_find_package(Pyrap REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
