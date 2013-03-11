# $Id$

lofar_package(OutputProc 1.0 DEPENDS Common ApplCommon Stream Interface MSLofar LofarStMan)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(DAL REQUIRED)
lofar_find_package(HDF5 REQUIRED)
lofar_find_package(Boost REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
