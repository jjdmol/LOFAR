# $Id$

include(LofarPackageVersion)

## ---------------------------------------------------------------------------
## Add MAC protocols (generates a .ph and .cc file from a .prot file)
## ---------------------------------------------------------------------------
include(MACAddProtocol)
mac_add_protocol(LOG_Protocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(KVT_Protocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(SB_Protocol  "${PACKAGE_SOURCE_DIR}/autogen")

## ---------------------------------------------------------------------------
## Add libraries to be generated
## ---------------------------------------------------------------------------
lofar_add_library(log_protocol LOG_Protocol.cc)
lofar_add_library(kvt_protocol KVT_Protocol.cc)
lofar_add_library(macio 
  Package__Version.cc
  GCF_Event.cc
  EventPort.cc
  KVT_Protocol.cc
  KVTLogger.cc
  ProtocolDefs.cc
  SB_Protocol.cc)

## ---------------------------------------------------------------------------
## Add executables to be generated
## ---------------------------------------------------------------------------
lofar_add_bin_program(versionmacio versionmacio.cc)
