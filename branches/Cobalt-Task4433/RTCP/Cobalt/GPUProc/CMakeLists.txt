# $Id: CMakeLists.txt 16350 2010-09-20 13:14:52Z nieuwpoort $

# We need at least CMake 2.8 for CUDA support
cmake_minimum_required(VERSION 2.8)

# Handle options USE_CUDA and USE_OPENCL.
if(USE_CUDA AND NOT USE_OPENCL)
  set(_gpuproc_deps Common Stream ApplCommon CoInterface InputProc)
  lofar_find_package(CUDA REQUIRED)
  add_definitions(-DUSE_CUDA)
  if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
    list(APPEND CUDA_NVCC_FLAGS "-g")
  endif()
elseif(USE_OPENCL AND NOT USE_CUDA)
  set(_gpuproc_deps Common Stream ApplCommon CoInterface InputProc OpenCL_FFT)
  lofar_find_package(OpenCL REQUIRED)
  add_definitions(-DUSE_OPENCL)
else()
  message(FATAL_ERROR
    "Either CUDA or OpenCL must be enabled to build GPUProc.")
endif()

lofar_package(GPUProc 1.0 DEPENDS ${_gpuproc_deps})

lofar_find_package(OpenMP REQUIRED)
lofar_find_package(LibSSH2 REQUIRED)
lofar_find_package(OpenSSL REQUIRED)
lofar_find_package(Boost REQUIRED)
lofar_find_package(MPI)
lofar_find_package(FFTW3 COMPONENTS single double threads)  # 'double threads' for FFT unit test refs
if(NOT FFTW3_FOUND)
  lofar_find_package(FFTW2 COMPONENTS single real)
  if(NOT FFTW2_FOUND)
    message(SEND_ERROR "Should have FFTW3 or FFTW2 installed.")
  endif(NOT FFTW2_FOUND)
endif(NOT FFTW3_FOUND)

# InputProc/Delays.h drags in a dependency on casa; hence we need to add the
# following line. This should be fixed someday.
lofar_find_package(Casacore COMPONENTS casa)

lofar_find_package(Valgrind)
if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

add_subdirectory(src)
add_subdirectory(test)
