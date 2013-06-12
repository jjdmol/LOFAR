# $Id$

include(LofarCTest)

# Add current source directory to -I path. This is needed because some
# source files and headers in subdirectories include UnitTest.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_test(tBestEffortQueue tBestEffortQueue.cc)
lofar_add_test(tSSH tSSH.cc)

# tests that use testParset.sh
lofar_add_test(tCorrelate_1sec_1st_5sb_noflagging DEPENDS rtcp)
lofar_add_test(tCorrelate_3sec_1st_5sb DEPENDS rtcp)
lofar_add_test(tCorrelate_3sec_2st_5sb DEPENDS rtcp)
#lofar_add_test(tGPUPerformance DEPENDS rtcp) # performance measurement and comparison are broken

add_subdirectory(Kernels)
add_subdirectory(Pipelines)
add_subdirectory(Storage)
add_subdirectory(WorkQueues)

if(USE_CUDA)
  add_subdirectory(cuda)
endif()

if(USE_OPENCL)
  add_subdirectory(opencl)
endif()
