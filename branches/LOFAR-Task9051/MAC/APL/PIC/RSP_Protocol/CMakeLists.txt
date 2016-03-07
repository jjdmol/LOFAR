# $Id$

lofar_package(RSP_Protocol 6.0 DEPENDS Common MACIO RTCCommon)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/RSP_Protocol)
add_subdirectory(src)
add_subdirectory(test)
