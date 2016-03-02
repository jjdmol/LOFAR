# $Id$

include(LofarCTest)

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_test(tPVSSservice tPVSSservice.cc Response.cc)
lofar_add_test(tPerformance tPerformance.cc PerformanceResponse.cc)
#lofar_add_test(tPVSSconnect tPVSSconnect.cc)
lofar_add_test(tPVSSinfo tPVSSinfo.cc Response.cc)
lofar_add_test(tGCFtypes tGCFtypes.cc)

if(BUILD_TESTING)
  set_tests_properties(
    tPVSSservice tPerformance tPVSSinfo
    PROPERTIES RUN_SERIAL ON)
endif(BUILD_TESTING)

# The following test program can only be run interactively. It monitors some
# data points in the PVSS data base for changes that must be made manually.
lofar_add_executable(tMonitorState tMonitorState.cc Response.cc)
