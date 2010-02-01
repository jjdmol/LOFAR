# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(CAL_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(cal_protocol
  Package__Version.cc
  CAL_Protocol.cc
  AntennaArray.cc
  AntennaArrayData.cc
  AntennaGains.cc
  SpectralWindow.cc
  SubArray.cc
  ACC.cc)

lofar_add_bin_program(versioncal_protocol versioncal_protocol.cc)
