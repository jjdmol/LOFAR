# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

include(LofarPackageVersion)

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(CR_Protocol "${MACIO_SOURCE_DIR}/autogen")

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/CR_Protocol)

lofar_add_library(cr_protocol
  Package__Version.cc
  CR_Protocol.cc
  CRstopRequest.cc
  CRstopVector.cc
  CRreadRequest.cc
  CRreadVector.cc
  CRrecordRequest.cc
  CRrecordVector.cc)

lofar_add_bin_program(versioncr_protocol versioncr_protocol.cc)

