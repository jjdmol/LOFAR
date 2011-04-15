# $Id$

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(Controller_Protocol "${MACIO_SOURCE_DIR}/autogen")
mac_add_protocol(StartDaemon_Protocol "${MACIO_SOURCE_DIR}/autogen")

lofar_add_library(aplcommon
  Package__Version.cc
  StartDaemon_Protocol.cc
  Controller_Protocol.cc
  APLUtilities.cc
  ControllerDefines.cc
  CTState.cc
  ControllerAdmin.cc
  ChildControl.cc
  ParentControl.cc)

lofar_add_bin_program(versionaplcommon versionaplcommon.cc)

install(PROGRAMS
  swlevel
  DESTINATION bin)

install(FILES
  swlevel.conf
  DESTINATION etc)
