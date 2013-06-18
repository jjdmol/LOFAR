# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorKernel tCorrelatorKernel)
  lofar_add_test(tDelayAndBandPassKernel2 tDelayAndBandPassKernel2)
  lofar_add_test(tFFT_Kernel tFFT_Kernel)
  lofar_add_test(tFIR_FilterKernel tFIR_FilterKernel)
endif()