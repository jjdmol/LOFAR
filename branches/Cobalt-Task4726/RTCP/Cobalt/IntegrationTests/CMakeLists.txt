# $Id$

lofar_package(CobaltIntegrationTests 1.2 DEPENDS GPUProc OutputProc)

include(LofarCTest)

lofar_add_test(tMACfeedback)
lofar_add_test(tProductionParsets)
lofar_add_test(tstartBGL)

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
