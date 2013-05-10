# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add current source directory to -I path. This is needed because some
# source files and headers in subdirectories include UnitTest.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Jan David, is this a test case? If so, please rename + lofar_add_test().
lofar_add_bin_program(DummyStorage DummyStorage.cc)

lofar_add_test(tBestEffortQueue tBestEffortQueue.cc)
lofar_add_test(tSSH tSSH.cc)
lofar_add_test(tStorageProcesses tStorageProcesses.cc)

if(USE_CUDA)
  add_subdirectory(cuda)
endif()

if(USE_OPENCL)
  add_subdirectory(opencl)
endif()
