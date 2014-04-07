# $Id$

include(LofarCTest)


lofar_add_test(tIntToFloatKernel tIntToFloatKernel.cc)
lofar_add_test(tDelayAndBandPassKernel tDelayAndBandPassKernel)
lofar_add_test(tBandPassCorrectionKernel tBandPassCorrectionKernel)
lofar_add_test(tBeamFormerKernel tBeamFormerKernel.cc)
lofar_add_test(tCoherentStokesKernel tCoherentStokesKernel.cc)
lofar_add_test(tFFTShiftKernel tFFTShiftKernel.cc)

# Compile some shallow wrapper executable to perform runtime compilation
lofar_add_executable(BeamFormerKernelPerformance BeamFormerKernelPerformance.cc)
lofar_add_test(tKernelPerformance)

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
  lofar_add_test(tCorrelatorKernel tCorrelatorKernel)
  lofar_add_test(tDelayAndBandPassKernel2 tDelayAndBandPassKernel2)
  lofar_add_test(tBandPassCorrectionKernel2 tBandPassCorrectionKernel2)
  lofar_add_test(tFFT_Kernel tFFT_Kernel)
  lofar_add_test(tFIR_FilterKernel tFIR_FilterKernel)
  lofar_add_test(tKernelFunctions tKernelFunctions)

  set_tests_properties(
    tCorrelatorKernel
    tDelayAndBandPassKernel2
    tBandPassCorrectionKernel2
    tFIR_FilterKernel
    tKernelFunctions
    PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
  )
endif()
