# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(ICAL_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(ical_protocol
  Package__Version.cc
  ICAL_Protocol.cc
  AntennaGains.cc
  SpectralWindow.cc
  SubArray.cc)
add_dependencies(ical_protocol ICAL_Protocol-ICAL_Protocol)

lofar_add_bin_program(versionical_protocol versionical_protocol.cc)
