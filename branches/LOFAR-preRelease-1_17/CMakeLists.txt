# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(ExpIon 1.0 DEPENDS pyparameterset pyparmdb)

include(LofarFindPackage)
lofar_find_package(Pyrap REQUIRED)
lofar_find_package(Boost REQUIRED COMPONENTS python thread)
lofar_find_package(Casacore REQUIRED COMPONENTS scimath)
add_subdirectory(src)
