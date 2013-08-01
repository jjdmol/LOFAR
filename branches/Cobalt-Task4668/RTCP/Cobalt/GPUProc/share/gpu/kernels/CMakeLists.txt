# $Id$

install(FILES 
  gpu_math.cuh
  IntToFloat.cuh
  BeamFormer.cu
  Correlator.cu
  DelayAndBandPass.cu
  FIR_Filter.cu
  IntToFloat.cu
  Transpose.cu
  DESTINATION share/gpu/kernels)

