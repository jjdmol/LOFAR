# $Id$

include(LofarCTest)

# TODO: move generic tests to ../ when supported by our CUDA port

lofar_add_test(tContext tContext.cc)
lofar_add_test(tPerformanceCounter tPerformanceCounter.cc)
#lofar_add_test(RTCP_UnitTest RTCP_UnitTest.cc UnitTest.cc)

lofar_find_package(UnitTest++)
if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorWorkQueue tCorrelatorWorkQueue.cc)
endif()

# tests that use testParset.sh
lofar_add_test(tCorrelate_1sec_1st_5sb_noflagging DEPENDS rtcp_opencl)
lofar_add_test(tCorrelate_3sec_1st_5sb DEPENDS rtcp_opencl)
lofar_add_test(tCorrelate_3sec_2st_5sb DEPENDS rtcp_opencl)
lofar_add_test(tGPUPerformance DEPENDS rtcp_opencl)
