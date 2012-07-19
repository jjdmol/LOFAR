# $Id$

lofar_package(Storage 1.0 DEPENDS Common ApplCommon Interface MSLofar LofarStMan)

set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures ms tables REQUIRED)
lofar_find_package(DAL REQUIRED)
lofar_find_package(HDF5 REQUIRED)

add_subdirectory(include/Storage)
add_subdirectory(src)
add_subdirectory(test)
