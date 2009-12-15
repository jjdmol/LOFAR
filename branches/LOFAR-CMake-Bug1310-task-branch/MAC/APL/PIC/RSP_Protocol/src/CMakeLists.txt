# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(EPA_Protocol "${MACIO_SOURCE_DIR}/autogen")
mac_add_protocol(RSP_Protocol "${MACIO_SOURCE_DIR}/autogen")
mac_add_protocol(RSPFE_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(rsp_protocol
  Package__Version.cc
  EPA_Protocol.cc
  RSP_Protocol.cc
  RSPFE_Protocol.cc
  AllRegisterState.cc
  BeamletWeights.cc
  BypassSettings.cc
  HBASettings.cc
  MEPHeader.cc
  MEPData.cc
  RCUSettings.cc
  RSUSettings.cc
  SPUStatus.cc
  Statistics.cc
  SubbandSelection.cc
  SystemStatus.cc
  TBBSettings.cc
  TDStatus.cc
  Versions.cc
  WGSettings.cc
  XCStatistics.cc)

lofar_add_bin_program(versionrsp_protocol versionrsp_protocol.cc)
