# $Id$

include(LofarCTest)

foreach(_file testFuncs.sh runtest.sh tStatusFeedback.run2)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${_file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${_file} @ONLY)
endforeach()

set(_tests
  tgenerateStationStreams
  tStatusFeedback
  tProductionParsets
  tstartBGL
  )

foreach(_test ${_tests})
  lofar_add_test(${_test})
  if(BUILD_TESTING)
    set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
  endif(BUILD_TESTING)
endforeach(_test ${_tests})

lofar_add_test(tRTmetadataToFile DEPENDS tRTmetadata PVSSGateway_Stub Service_Broker)
if(BUILD_TESTING)
  set_tests_properties(tRTmetadataToFile PROPERTIES RUN_SERIAL ON)
endif(BUILD_TESTING)

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
add_subdirectory(Commensal)
