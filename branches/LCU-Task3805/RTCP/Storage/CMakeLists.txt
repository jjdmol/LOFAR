# $Id$

lofar_package(Storage 1.0 DEPENDS Common ApplCommon Stream Interface MSLofar LofarStMan)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(DAL REQUIRED)
lofar_find_package(HDF5 REQUIRED)

add_subdirectory(include/Storage)
add_subdirectory(src)
add_subdirectory(test)
