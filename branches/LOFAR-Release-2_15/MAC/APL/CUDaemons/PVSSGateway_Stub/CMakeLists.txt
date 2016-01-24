# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

lofar_package(PVSSGateway_Stub 1.0 DEPENDS Common GCFTM MACIO)
# Add current directory to the -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(LofarPackageVersion)

lofar_add_library(pvssgwstub
  Package__Version.cc)

lofar_add_bin_program(PVSSGatewayStub PVSSGatewayStubMain.cc PVSSGatewayStub.cc)

install(FILES PVSSGatewayStub.conf DESTINATION etc)
