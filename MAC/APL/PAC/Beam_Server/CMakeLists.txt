# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(Beam_Server 6.0 DEPENDS Common AMCBase MACIO GCFTM TestSuite RTCCommon RSP_Protocol BS_Protocol CAL_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_definitions(-DUSE_CAL_THREAD)

add_subdirectory(src)
add_subdirectory(test)
