# $Id$

include(LofarCTest)

if(USE_CUDA AND NOT USE_OPENCL)
  # We need at least CMake 2.8 for CUDA support
  cmake_minimum_required(VERSION 2.8)

  lofar_find_package(OpenMP REQUIRED)
  lofar_find_package(CUDA 4.1 REQUIRED)
  lofar_find_package(CUDADriver REQUIRED)

  add_definitions(-DUSE_CUDA)
  if(LOFAR_BUILD_VARIANT MATCHES "^DEBUG$")
    list(APPEND CUDA_NVCC_FLAGS -g -G)
  endif()

  lofar_add_test(tMultiPartTABOutput tMultiPartTABOutput.cc)
  lofar_add_test(tManyPartTABOutput tManyPartTABOutput.cc)
endif()

foreach(_file testFuncs runtest)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${_file}.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/${_file}.sh @ONLY)
endforeach()

set(_tests
  tgenerateStationStreams
  tMACfeedback
  tProductionParsets
  tstartBGL
)

foreach(_test ${_tests})
  lofar_add_test(${_test})
  set_tests_properties(${_test} PROPERTIES RUN_SERIAL ON)
endforeach(_test ${_tests})

add_subdirectory(Correlator)
add_subdirectory(Beamformer)
