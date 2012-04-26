# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(ITRFBeamServer 1.0 DEPENDS Common ApplCommon MACIO GCFTM APLCommon RTCCommon RSP_Protocol IBS_Protocol CAL_Protocol CASATools)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa measures)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
