# $Id: CMakeLists.txt 17975 2011-05-10 09:52:51Z mol $

lofar_package(OpenCL_FFT 1.0)

include(LofarFindPackage)

if(USE_CUDA)
  # Use NVidia's libOpenCL when using CUDA.
  set(ENV{OPENCL_LIB} $ENV{CUDA_LIB_PATH})
endif()

lofar_find_package(OpenCL REQUIRED)

if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

add_subdirectory(src)
