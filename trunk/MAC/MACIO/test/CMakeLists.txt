# $Id$

include(LofarCTest)

include(MACAddProtocol)

mac_add_protocol(Echo_Protocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(testprotocol "${PACKAGE_SOURCE_DIR}/autogen")
mac_add_protocol(Clone_Protocol "${PACKAGE_SOURCE_DIR}/autogen")

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_test(tEventPort tEventPort.cc Echo_Protocol.cc)
lofar_add_test(tProtocol tProtocol.cc Echo_Protocol.cc testprotocol.cc)
lofar_add_test(tClone tClone.cc Clone_Protocol.cc)
