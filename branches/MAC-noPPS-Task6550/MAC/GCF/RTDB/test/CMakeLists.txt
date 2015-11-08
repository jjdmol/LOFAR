# $Id$

include(LofarCTest)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/testFuncs.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/testFuncs.sh @ONLY)

lofar_add_test(tPropertySet tPropertySet.cc)
lofar_add_test(tRTDBPerformance tRTDBPerformance.cc RTDBPerfResp.cc)
lofar_add_test(tDPservice tDPservice.cc DPresponse.cc)
lofar_add_test(tRTDBPort tRTDBPort.cc)

if(BUILD_TESTING)
  set_tests_properties(
    tPropertySet tRTDBPerformance tDPservice tRTDBPort
    PROPERTIES RUN_SERIAL ON)
endif(BUILD_TESTING)

# The following tests can only be run interactively. Here we only build them.
lofar_add_executable(testQuery testQuery.cc)
lofar_add_executable(tPerformanceReport tPerformanceReport.cc RTDBPerfResp.cc)
