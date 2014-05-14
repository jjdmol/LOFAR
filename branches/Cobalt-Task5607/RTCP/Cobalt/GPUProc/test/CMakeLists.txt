# $Id$

include(LofarCTest)

# Add current source directory to -I path. This is needed because some
# source files and headers in subdirectories include UnitTest.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_test(tBandPass tBandPass.cc)
lofar_add_test(tSSH tSSH.cc)
lofar_add_test(t_cpu_utils t_cpu_utils.cc)
lofar_add_test(tMPISendReceive tMPISendReceive.cc)
if(UNITTEST++_FOUND)
  lofar_add_test(t_gpu_utils t_gpu_utils)
  lofar_add_test(tStationInput tStationInput)
  
endif()

add_subdirectory(Kernels)
add_subdirectory(Pipelines)
add_subdirectory(Storage)
add_subdirectory(SubbandProcs)

if(USE_CUDA)
  add_subdirectory(cuda)
endif()

if(USE_OPENCL)
  add_subdirectory(opencl)
endif()
