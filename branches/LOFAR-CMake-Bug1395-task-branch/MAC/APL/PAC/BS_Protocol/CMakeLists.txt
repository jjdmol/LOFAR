# $Id$

lofar_package(BS_Protocol 6.0 DEPENDS Common AMCBase MACIO RTCCommon RSP_Protocol)

add_subdirectory(include/APL/BS_Protocol)
add_subdirectory(src)
add_subdirectory(test)
