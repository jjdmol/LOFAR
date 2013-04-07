# $Id: CMakeLists.txt 16350 2010-09-20 13:14:52Z nieuwpoort $

lofar_package(GPUProc 1.0 DEPENDS Common Stream ApplCommon CoInterface)

#add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(OpenMP REQUIRED)

if(USE_CUDA)
  lofar_find_package(CUDA 4.1 REQUIRED)
  add_definitions(-DUSE_CUDA)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/cuda)

  set(HAVE_CUDA ON)

  #message("CUDA_FOUND              = ${CUDA_FOUND}")
  #message("CUDA_VERSION            = ${CUDA_VERSION}")
  #message("CUDA_TOOLKIT_ROOT_DIR   = ${CUDA_TOOLKIT_ROOT_DIR}")
  #message("CUDA_SDK_ROOT_DIR       = ${CUDA_SDK_ROOT_DIR}")  # if N/A, CUDA_SDK_ROOT_DIR-NOTFOUND

  # These are not set with cmake 2.8.9 detecting CUDA 5.0 for some reason. Maybe elsewhere?
  #message("CUDA_VERSION_MAJOR      = ${CUDA_VERSION_MAJOR}")
  #message("CUDA_VERSION_MINOR      = ${CUDA_VERSION_MINOR}")
  #message("CUDA_VERSION_STRING     = ${CUDA_VERSION_STRING}")
  #message("CUDA_INCLUDE_DIRS       = ${CUDA_INCLUDE_DIRS}")
  #message("CUDA_LIBRARIES          = ${CUDA_LIBRARIES}")
  #message("CUDA_CUFFT_LIBRARIES    = ${CUDA_CUFFT_LIBRARIES}")
  #message("CUDA_CUBLAS_LIBRARIES   = ${CUDA_CUBLAS_LIBRARIES}")
else(USE_CUDA)
  set(HAVE_CUDA OFF)
endif(USE_CUDA)

if(USE_OPENCL)
  lofar_add_package(OpenCL_FFT)  # extra dep of GPUProc
  lofar_find_package(OpenCL REQUIRED)
  add_definitions(-DUSE_OPENCL) # USE_CUDA still active? idem for include dir
  
  if(USE_PATCHED_OPENCL_HEADERS)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/backward)
  endif(USE_PATCHED_OPENCL_HEADERS)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/opencl)
  #include_directories(clAmdFft/include) # AMD OpenCL FFT

  set(HAVE_OPENCL ON)
else(USE_OPENCL)
  set(HAVE_OPENCL OFF)
endif(USE_OPENCL)
lofar_find_package(LibSSH2 REQUIRED)
lofar_find_package(OpenSSL REQUIRED)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(Casacore COMPONENTS measures REQUIRED)
lofar_find_package(FFTW3 COMPONENTS single double threads)  # 'double threads' for FFT unit test refs
if(NOT FFTW3_FOUND)
  lofar_find_package(FFTW2 COMPONENTS single real)
  if(NOT FFTW2_FOUND)
    message(SEND_ERROR "Should have FFTW3 or FFTW2 installed.")
  endif(NOT FFTW2_FOUND)
endif(NOT FFTW3_FOUND)

lofar_find_package(Valgrind)
if(USE_VALGRIND)
  add_definitions(-DUSE_VALGRIND)
endif(USE_VALGRIND)  

add_subdirectory(src)
add_subdirectory(test)
