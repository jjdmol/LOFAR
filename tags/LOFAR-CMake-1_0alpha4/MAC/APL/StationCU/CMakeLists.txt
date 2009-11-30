# $Id$

lofar_package(StationCU 1.0 DEPENDS 
  Common ApplCommon MACIO GCFTM GCFRTDB GCFPVSS APLCommon RTCCommon RTDBCommon
  BS_Protocol CAL_Protocol RSP_Protocol TBB_Protocol VHECR)

include(LofarFindPackage)
lofar_find_package(Blitz REQUIRED)

add_subdirectory(include/APL/ClockProtocol)
add_subdirectory(src)
