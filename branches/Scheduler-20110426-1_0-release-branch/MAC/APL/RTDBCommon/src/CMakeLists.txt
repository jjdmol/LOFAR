# $Id$

include(LofarPackageVersion)

# Extra preprocessor definitions
add_definitions(-DUSES_NEW_IOSTREAMS)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(CM_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(rtdbcommon
  Package__Version.cc
  CM_Protocol.cc
  RTDButilities.cc
  ClaimMgrTask.cc)

lofar_add_bin_program(versionrtdbcommon versionrtdbcommon.cc)
