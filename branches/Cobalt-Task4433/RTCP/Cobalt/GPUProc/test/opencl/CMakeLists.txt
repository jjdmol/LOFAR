# $Id$

include(LofarCTest)

lofar_add_test(tContext tContext.cc)
lofar_add_test(tPerformanceCounter tPerformanceCounter.cc)

lofar_find_package(UnitTest++)
if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorWorkQueue 
    UnitTests/WorkQueues/tCorrelatorWorkQueue.cc
    UnitTests/new_style_unittest.cc)
endif()
