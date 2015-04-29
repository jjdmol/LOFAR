# $Id: CMakeLists.txt 27640 2013-12-04 08:02:49Z diepen $

lofar_package(AOFlaggerOld 1.0 DEPENDS DPPP AOFlagger)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)

add_subdirectory(include/AOFlaggerOld)
add_subdirectory(src)
add_subdirectory(test)
