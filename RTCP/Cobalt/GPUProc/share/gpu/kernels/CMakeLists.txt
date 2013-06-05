# $Id$

install(FILES 
  complex.cuh 
  gpu_math.cuh
  FIR_Filter.cu
  DelayAndBandPass.cu
  Correlator.cu
  DESTINATION share/gpu/kernels)
