# $Id$

include(LofarCTest)

set(_tests
  tMACfeedback
  tProductionParsets
  tstartBGL
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
endforeach(_test ${_tests})

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
