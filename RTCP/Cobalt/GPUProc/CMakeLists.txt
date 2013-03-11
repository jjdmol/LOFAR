# $Id: CMakeLists.txt 16350 2010-09-20 13:14:52Z nieuwpoort $

lofar_package(GPUProc 1.0 DEPENDS Common Stream ApplCommon Interface OpenCL_FFT)

#add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)

include(LofarFindPackage)
lofar_find_package(OpenMP REQUIRED)
lofar_find_package(OpenCL REQUIRED)
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

if(USE_PATCHED_OPENCL_HEADERS)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/backward)
endif(USE_PATCHED_OPENCL_HEADERS)

#include_directories(clAmdFft/include) # AMD OpenCL FFT

add_subdirectory(src)
add_subdirectory(test)
