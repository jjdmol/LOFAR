# $Id$

lofar_package(Storage 1.0 DEPENDS Common AMCBase Interface)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures ms tables REQUIRED)

add_subdirectory(include/Storage)
add_subdirectory(src)
add_subdirectory(test)
