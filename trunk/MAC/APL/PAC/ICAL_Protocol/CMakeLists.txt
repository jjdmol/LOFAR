# $Id$

lofar_package(ICAL_Protocol 1.0 DEPENDS Common MACIO RTCCommon APLCommon RSP_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/ICAL_Protocol)
add_subdirectory(src)
