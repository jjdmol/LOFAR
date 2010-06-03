# $Id: CMakeLists.txt 14280 2009-10-16 13:48:46Z loose $

lofar_package(LBA_Calibration 1.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Matlab REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
