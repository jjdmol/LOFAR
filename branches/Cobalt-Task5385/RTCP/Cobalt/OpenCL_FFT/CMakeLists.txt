# $Id$

lofar_package(OpenCL_FFT 1.0)

if(USE_OPENCL)
  include(LofarFindPackage)
  lofar_find_package(OpenCL REQUIRED)
  add_subdirectory(src)
endif()
