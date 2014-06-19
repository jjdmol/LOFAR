# $Id$

include(LofarCTest)


lofar_add_test(tIntToFloatKernel tIntToFloatKernel.cc)
lofar_add_test(tDelayAndBandPassKernel tDelayAndBandPassKernel.cc KernelTestHelpers.cc)
lofar_add_test(tBandPassCorrectionKernel tBandPassCorrectionKernel.cc)
lofar_add_test(tBeamFormerKernel tBeamFormerKernel.cc KernelTestHelpers.cc)
lofar_add_test(tCoherentStokesKernel tCoherentStokesKernel.cc KernelTestHelpers.cc)
lofar_add_test(tFFTShiftKernel tFFTShiftKernel.cc)

# This test uses quite a lot of memory, so force it to run serially.
set_tests_properties(tCoherentStokesKernel PROPERTIES RUN_SERIAL ON)

# Compile some shallow wrapper executable to perform runtime compilation

# Test that run for a long time and are use for performance optimalization
# lofar_add_test(tKernelPerformance)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
set_tests_properties(
  tIntToFloatKernel
  tFFTShiftKernel
  tDelayAndBandPassKernel
  tBandPassCorrectionKernel
  tBeamFormerKernel
  tCoherentStokesKernel
  PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorKernel tCorrelatorKernel.cc)
  lofar_add_test(tDelayAndBandPassKernel2 tDelayAndBandPassKernel2.cc)
  lofar_add_test(tBandPassCorrectionKernel2 tBandPassCorrectionKernel2.cc)
  lofar_add_test(tFFT_Kernel tFFT_Kernel.cc)
  lofar_add_test(tFIR_FilterKernel tFIR_FilterKernel.cc)
  lofar_add_test(tKernelFunctions tKernelFunctions.cc)

  set_tests_properties(
    tCorrelatorKernel
    tDelayAndBandPassKernel2
    tBandPassCorrectionKernel2
    tFIR_FilterKernel
    tKernelFunctions
    PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
  )
endif()
