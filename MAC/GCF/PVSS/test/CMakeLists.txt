# $Id$

include(LofarCTest)

# Add current binary directory to -I path, so that the generated C++ header
# files can be found.
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_test(tPVSSservice tPVSSservice.cc Response.cc)
lofar_add_test(tPerformance tPerformance.cc PerformanceResponse.cc)
#lofar_add_test(tPVSSconnect tPVSSconnect.cc)
lofar_add_test(tMonitorState tMonitorState.cc Response.cc)
lofar_add_test(tPVSSinfo tPVSSinfo.cc Response.cc)
