# $Id$

include(LofarCTest)

foreach(_file testFuncs runtest)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${_file}.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/${_file}.sh @ONLY)
endforeach()

set(_tests
  tgenerateStationStreams
  tMACfeedback
  tProductionParsets
  tstartBGL
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
endforeach(_test ${_tests})

lofar_add_test(tRTmetadataToFile DEPENDS tRTmetadata PVSSGateway_Stub Service_Broker)
set_tests_properties(tRTmetadataToFile PROPERTIES RUN_SERIAL ON)

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
