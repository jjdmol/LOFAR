# $Id$

install(FILES 
  complex.cuh 
  gpu_math.cuh
  IntToFloat.cuh
  IntToFloat.cu
  FIR_Filter.cu
  DelayAndBandPass.cu
  Correlator.cu
  DESTINATION share/gpu/kernels)
