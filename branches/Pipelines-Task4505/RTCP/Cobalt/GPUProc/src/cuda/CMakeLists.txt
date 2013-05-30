# $Id$

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cuda_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cuda_config.h"  # internal, no need to install
)

install(FILES 
  complex.h 
  gpu_math.cuh
  Correlator.cu
  DelayAndBandPass.cu
  FIR_Filter.cu
  DESTINATION share/gpu/kernels)
