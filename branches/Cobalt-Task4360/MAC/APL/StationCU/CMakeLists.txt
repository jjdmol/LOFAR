# $Id$

lofar_package(StationCU 1.0 DEPENDS 

Common ApplCommon MACIO GCFTM GCFRTDB GCFPVSS APLCommon RTCCommon RTDBCommon
IBS_Protocol CAL_Protocol RSP_Protocol TBB_Protocol VHECR CR_Protocol)


include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)
lofar_find_package(Boost REQUIRED date_time)

add_subdirectory(include/APL/ClockProtocol)
add_subdirectory(src)
