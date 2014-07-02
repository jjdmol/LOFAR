# $Id$

# Add current source directory to -I path. This is needed because GCFTM uses
# angle brackets for internal header files, instead of quotes.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

# Add MAC protocol(s)
include(MACAddProtocol)
mac_add_protocol(TestFW_Protocol "${MACIO_SOURCE_DIR}/autogen")

lofar_add_library(testFW
  TestFW_Protocol.cc
  TestTask.cc)
add_dependencies(testFW GCFTM-TestFW_Protocol)
