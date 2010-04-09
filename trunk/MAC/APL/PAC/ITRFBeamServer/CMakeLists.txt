# $Id: CMakeLists.txt 14697 2009-12-16 15:06:53Z loose $

# Do not split the following line, otherwise makeversion will fail!
lofar_package(ITRFBeamServer 1.0 DEPENDS Common ApplCommon MACIO GCFTM RTCCommon RSP_Protocol IBS_Protocol CAL_Protocol)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_definitions(-DUSE_CAL_THREAD)

add_subdirectory(src)
#add_subdirectory(test)
