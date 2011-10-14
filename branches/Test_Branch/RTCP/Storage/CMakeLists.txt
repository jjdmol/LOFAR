# $Id$

lofar_package(Storage 1.0 DEPENDS Common AMCBase Interface MSLofar LofarStMan)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures ms tables REQUIRED)
lofar_find_package(DAL)
lofar_find_package(LDA)
lofar_find_package(WCSLIB)
lofar_find_package(HDF5)

add_subdirectory(include/Storage)
add_subdirectory(src)
add_subdirectory(test)
