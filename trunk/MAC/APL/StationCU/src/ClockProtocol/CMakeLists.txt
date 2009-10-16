# $Id$

# Add MAC protocols (generates a .ph and .cc file from a .prot file)
include(MACAddProtocol)
mac_add_protocol(Clock_Protocol "${MACIO_SOURCE_DIR}/autogen")

lofar_add_library(clock_protocol Clock_Protocol.cc)
