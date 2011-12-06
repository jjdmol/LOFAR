# $Id$

lofar_package(LofarGridder 0.1 DEPENDS BBSKernel Common ParmDB askap-synthesis)

lofar_find_package(Casacore REQUIRED COMPONENTS casa measures ms tables)

add_subdirectory(include/LofarGridder)
add_subdirectory(src)
add_subdirectory(test)
