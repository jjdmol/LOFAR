# $Id: CMakeLists.txt 17975 2011-05-10 09:52:51Z mol $

lofar_package(OpenCL_FFT 1.0)

if(USE_OPENCL)
  include(LofarFindPackage)
  lofar_find_package(OpenCL REQUIRED)
  add_subdirectory(src)
endif()
