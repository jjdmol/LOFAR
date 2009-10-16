# $Id$

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(EC_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add directory with the generated protocol files to the -I path.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_bin_program(HardwareMonitor 
  HardwareMonitorMain.cc
  RSPMonitor.cc
  TBBMonitor.cc
  ECMonitor.cc
  EC_Protocol.cc)

install(FILES
  HardwareMonitor.conf
  DESTINATION etc)