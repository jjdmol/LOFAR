# $Id: CMakeLists.txt 16350 2010-09-20 13:14:52Z nieuwpoort $

# We need at least CMake 2.8 for CUDA support
cmake_minimum_required(VERSION 2.8)

lofar_package(GPUProc 1.0 DEPENDS Common Stream ApplCommon CoInterface InputProc)

# Handle options USE_CUDA and USE_OPENCL.
if(NOT USE_CUDA AND NOT USE_OPENCL)
  message(FATAL_ERROR
    "You need to enable USE_CUDA or USE_OPENCL to build ${PACKAGE_NAME}. "
    "Please check your variants file!")
endif()

# If USE_CUDA and USE_OPENCL are both defined, then build two separate binaries.
# Do not try to build source with -DUSE_CUDA and -DUSE_OPENCL at once (will #error).

if(USE_CUDA)
  lofar_find_package(CUDA 4.1 REQUIRED)

  if(0)  # for dev to see what is available how; disabled to reduce spam
    message(STATUS "CUDA_FOUND = ${CUDA_FOUND}")
    message(STATUS "CUDA_VERSION_MAJOR = ${CUDA_VERSION_MAJOR}")
    message(STATUS "CUDA_VERSION_MINOR = ${CUDA_VERSION_MINOR}")
    message(STATUS "CUDA_VERSION = ${CUDA_VERSION}") # sometimes avail if CUDA_VERSION_STRING is not
    message(STATUS "CUDA_VERSION_STRING = ${CUDA_VERSION_STRING}")
    message(STATUS "CUDA_TOOLKIT_ROOT_DIR = ${CUDA_TOOLKIT_ROOT_DIR}")
    message(STATUS "CUDA_SDK_ROOT_DIR = ${CUDA_SDK_ROOT_DIR}")
    message(STATUS "CUDA_INCLUDE_DIRS = ${CUDA_INCLUDE_DIRS}")
    message(STATUS "CUDA_LIBRARIES = ${CUDA_LIBRARIES}")
    message(STATUS "CUDA_CUFFT_LIBRARIES = ${CUDA_CUFFT_LIBRARIES}")
    message(STATUS "CUDA_CUBLAS_LIBRARIES = ${CUDA_CUBLAS_LIBRARIES}")
    message(STATUS "CUDA_curand_LIBRARY = ${CUDA_curand_LIBRARY}")
    message(STATUS "CUDA_cusparse_LIBRARY = ${CUDA_cusparse_LIBRARY}")
    message(STATUS "CUDA_npp_LIBRARY = ${CUDA_npp_LIBRARY}")
    message(STATUS "CUDA_nvcuvenc_LIBRARY = ${CUDA_nvcuvenc_LIBRARY}")
    message(STATUS "CUDA_nvcuvid_LIBRARY = ${CUDA_nvcuvid_LIBRARY}")
  endif()

  if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
    list(APPEND CUDA_NVCC_FLAGS "-g")
  endif()
  # For backward compatibility with existing OpenCL code
  lofar_find_package(OpenCL)
endif()

if(USE_OPENCL)
  lofar_add_package(OpenCL_FFT)
  lofar_find_package(OpenCL REQUIRED)
endif()

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED)
lofar_find_package(OpenMP REQUIRED)
lofar_find_package(LibSSH2 REQUIRED)
lofar_find_package(OpenSSL REQUIRED)
lofar_find_package(MPI REQUIRED)
lofar_find_package(FFTW3 COMPONENTS single double threads REQUIRED)  # 'double threads' for FFT unit test refs

# InputProc/Delays.h drags in a dependency on casa; hence we need to add the
# following line. This should be fixed someday.
lofar_find_package(Casacore COMPONENTS casa)

if(USE_VALGRIND)
  lofar_find_package(Valgrind)
  if(VALGRIND_FOUND)
    add_definitions(-DUSE_VALGRIND)
  else()
    message(WARNING "USE_VALGRIND requested, but not found!")
  endif()
endif()  

add_subdirectory(src)
add_subdirectory(test)
