# $Id$

lofar_add_bin_program(SASGateway SASGatewayMain.cc SASGateway.cc)

install(FILES SASGateway_prod.conf SASGateway_test.conf DESTINATION etc)
