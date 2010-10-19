# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(SHMInfo_Server 6.0 DEPENDS Common MACIO GCFTM GCFRTDB GCFPVSS RTCCommon RTDBCommon RSP_Protocol CAL_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_definitions(-DUSES_NEW_IOSTREAM)

add_subdirectory(src)
add_subdirectory(test)
