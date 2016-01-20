# $Id: CMakeLists.txt 14280 2009-10-16 13:48:46Z loose $

lofar_package(CR_Protocol 1.0 DEPENDS MACIO RTCCommon)

add_subdirectory(include/APL/CR_Protocol)
add_subdirectory(src)
# add_subdirectory(test)
