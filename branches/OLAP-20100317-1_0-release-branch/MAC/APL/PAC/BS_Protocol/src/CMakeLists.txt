# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(BS_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(bs_protocol
  Package__Version.cc
  BS_Protocol.cc
  Beamlet2SubbandMap.cc
  Pointing.cc)

lofar_add_bin_program(versionbs_protocol versionbs_protocol.cc)
