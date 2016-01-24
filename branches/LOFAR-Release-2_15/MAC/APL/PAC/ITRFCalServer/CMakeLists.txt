# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(ITRFCalServer 1.0 DEPENDS Common ApplCommon MACIO GCFTM APLCommon RTCCommon RSP_Protocol ICAL_Protocol LBA_Calibration)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures)
lofar_find_package(Blitz REQUIRED)

#add_definitions(-DUSE_CAL_THREAD)

add_subdirectory(src)
add_subdirectory(test)
