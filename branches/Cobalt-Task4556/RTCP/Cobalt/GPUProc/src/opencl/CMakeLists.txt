# $Id$

install(FILES
  Correlator.cl
  DelayAndBandPass.cl
  #fft2.cl
  FFT.cl
  FIR.cl
  math.cl
  NewCorrelator.cl
  DESTINATION share/gpu/kernels)
