# $Id$

include(LofarCTest)

lofar_add_test(tAntField tAntField.cc)
lofar_add_test(tAntennaSet tAntennaSet.cc)
lofar_add_test(tObservation tObservation.cc)
lofar_add_executable(tHasDataslots tHasDataslots.cc)
lofar_add_executable(tStationInfo tStationInfo.cc)
