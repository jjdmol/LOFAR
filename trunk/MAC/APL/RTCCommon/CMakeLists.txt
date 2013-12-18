# $Id$

lofar_package(RTCCommon 6.0 DEPENDS Common)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/RTCCommon)
add_subdirectory(src)
add_subdirectory(test)
