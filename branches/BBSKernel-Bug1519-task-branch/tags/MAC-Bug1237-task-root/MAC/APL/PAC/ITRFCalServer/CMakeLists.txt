# $Id: CMakeLists.txt 14280 2009-10-16 13:48:46Z loose $

# Do not split the following line, otherwise makeversion will fail!
lofar_package(Cal_Server 6.0 DEPENDS Common AMCBase ApplCommon MACIO GCFTM RTCCommon RSP_Protocol ICAL_Protocol LBA_Calibration)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_definitions(-DUSE_CAL_THREAD)

add_subdirectory(src)
add_subdirectory(test)
