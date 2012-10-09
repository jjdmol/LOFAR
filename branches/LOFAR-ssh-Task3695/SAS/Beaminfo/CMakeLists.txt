# $Id: CMakeLists.txt 16887 2011-09-20 10:46:43Z duscha $

lofar_package(Beaminfo 0.1 DEPENDS Common ApplCommon MSLofar OTDB) 

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(Boost COMPONENTS date_time REQUIRED)

add_subdirectory(include/Beaminfo)
add_subdirectory(src)
add_subdirectory(test)
