# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add current source directory to -I path. This is needed because some
# source files and headers in subdirectories include UnitTest.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_test(tBestEffortQueue tBestEffortQueue.cc)
lofar_add_test(tCorrelate_1sec_1st_5sb_noflagging)
lofar_add_test(tCorrelate_3sec_1st_5sb)
lofar_add_test(tCorrelate_3sec_2st_5sb)
lofar_add_test(tGPUPerformance)

lofar_add_test(tSSH tSSH.cc)
lofar_add_bin_program(DummyStorage DummyStorage.cc)
lofar_add_test(tStorageProcesses tStorageProcesses.cc)
#lofar_add_test(tDelayCompensation tDelayCompensation.cc)

lofar_find_package(UnitTest++)

if(USE_CUDA)
  add_subdirectory(cuda)
endif()

if(USE_OPENCL)
  lofar_add_test(RTCP_UnitTest RTCP_UnitTest.cc UnitTest.cc)
  lofar_add_test(tContext tContext.cc)
  lofar_add_test(tPerformanceCounter tPerformanceCounter.cc)
  if(UNITTEST++_FOUND)
    lofar_add_test(tCorrelatorWorkQueue 
      UnitTests/WorkQueues/CorrelatorWorkQueue.cc
      UnitTests/new_style_unittest.cc)
  endif()
endif()
