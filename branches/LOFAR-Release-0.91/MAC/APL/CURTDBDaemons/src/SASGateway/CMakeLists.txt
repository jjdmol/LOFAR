# $Id$

lofar_add_bin_program(SASGateway SASGatewayMain.cc SASGateway.cc)

install(FILES SASGateway.conf DESTINATION etc)
