# $Id$

lofar_package(CAL_Protocol 6.0 DEPENDS Common MACIO RTCCommon RSP_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/CAL_Protocol)
add_subdirectory(src)
#add_subdirectory(test)
