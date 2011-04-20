# $Id$

lofar_package(MSLofar 0.1 DEPENDS ApplCommon Common)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures ms tables REQUIRED)

add_subdirectory(include/MSLofar)
add_subdirectory(src)
add_subdirectory(test)
