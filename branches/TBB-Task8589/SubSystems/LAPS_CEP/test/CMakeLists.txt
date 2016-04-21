# $Id$

include(LofarCTest)

foreach(_file testFuncs )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${_file}.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/${_file}.sh @ONLY)
endforeach()

lofar_add_test(tExampleTest)
lofar_add_test(tBasicQueueTest)
lofar_add_test(startPythonFromMsg)

# set(_tests
  # tgenerateStationStreams
  # tMACfeedback
  # tProductionParsets
  # tstartBGL
# )

# foreach(_test ${_tests})
  # lofar_add_test(${_test})
  # if(BUILD_TESTING)
  #   set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
  # endif(BUILD_TESTING)
# endforeach(_test ${_tests})