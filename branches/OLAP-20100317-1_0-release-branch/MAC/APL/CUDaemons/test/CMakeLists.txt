# $Id$

include(LofarCTest)

lofar_add_test(LDtestMenu LDtestMenu.cc EventPort.cc)
lofar_add_test(ObservationControl dummyLD.cc)
lofar_add_test(tLogClient tLogClient.cc)
