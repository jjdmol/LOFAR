# $Id$

lofar_package(SPW_Combine 1.0 DEPENDS Common PLC)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)

add_subdirectory(include/SPWCombine)
add_subdirectory(src)
#add_subdirectory(test)
