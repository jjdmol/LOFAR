# $Id$

install(FILES 
  complex.cuh 
  gpu_math.cuh
  Correlator.cu
  DelayAndBandPass.cu
  FIR_Filter.cu
  DESTINATION share/gpu/kernels)
