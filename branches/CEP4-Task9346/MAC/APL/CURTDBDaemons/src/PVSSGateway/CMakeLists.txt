# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

install(FILES
  PVSSGateway.conf
  DESTINATION etc)

lofar_add_bin_program(PVSSGateway PVSSGatewayMain.cc PVSSGateway.cc)
