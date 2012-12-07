# $Id: CMakeLists.txt 16350 2010-09-20 13:14:52Z nieuwpoort $

lofar_package(GPUProc 1.0 DEPENDS Common Stream Interface OpenCL_FFT)

#add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)

include(LofarFindPackage)
lofar_find_package(OpenMP REQUIRED)
lofar_find_package(OpenCL REQUIRED)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS measures REQUIRED)
lofar_find_package(FFTW3 COMPONENTS single)
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
