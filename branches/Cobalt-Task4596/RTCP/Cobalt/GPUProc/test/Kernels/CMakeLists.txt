# $Id$

include(LofarCTest)


lofar_add_test(tIntToFloatKernel tIntToFloatKernel.cc)
lofar_add_test(tDelayAndBandPassKernel tDelayAndBandPassKernel)
lofar_add_test(tBeamFormerKernel tBeamFormerKernel.cc)
lofar_add_test(tCoherentStokesKernel tCoherentStokesKernel.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tIntToFloatKernel
  tDelayAndBandPassKernel
  tBeamFormerKernel
  tCoherentStokesKernel
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorKernel tCorrelatorKernel)
  lofar_add_test(tDelayAndBandPassKernel2 tDelayAndBandPassKernel2)
  lofar_add_test(tFFT_Kernel tFFT_Kernel)
  lofar_add_test(tFIR_FilterKernel tFIR_FilterKernel)

  set_tests_properties(
    tCorrelatorKernel
    tDelayAndBandPassKernel2
    tFIR_FilterKernel
    PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
  )
endif()

