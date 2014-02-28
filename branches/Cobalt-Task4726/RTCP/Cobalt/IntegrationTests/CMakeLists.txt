# $Id$

lofar_package(CobaltIntegrationTests 1.2 DEPENDS GPUProc OutputProc)

include(LofarCTest)

lofar_add_test(tMACfeedback DEPENDS rtcp outputProc)
lofar_add_test(tProductionParsets DEPENDS rtcp outputProc)
lofar_add_test(tstartBGL DEPENDS rtcp outputProc)

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
