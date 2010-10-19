# $Id: CMakeLists.txt 14280 2009-10-16 13:48:46Z loose $

lofar_package(ICAL_Protocol 1.0 DEPENDS Common MACIO RTCCommon RSP_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/ICAL_Protocol)
add_subdirectory(src)
