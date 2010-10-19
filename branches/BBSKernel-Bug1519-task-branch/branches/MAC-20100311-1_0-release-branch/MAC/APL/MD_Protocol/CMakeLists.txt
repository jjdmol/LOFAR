# $Id$

lofar_package(MD_Protocol 6.0 DEPENDS Common MACIO RTCCommon)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/MD_Protocol)
add_subdirectory(src)
#add_subdirectory(test)
