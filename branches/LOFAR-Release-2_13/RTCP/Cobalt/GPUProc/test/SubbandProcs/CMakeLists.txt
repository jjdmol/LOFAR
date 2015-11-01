# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorSubbandProc tCorrelatorSubbandProc.cc)
  lofar_add_test(tCorrelatorStep tCorrelatorStep.cc)
endif()

# This test is instable. Added to issue tracker: https://support.astron.nl/lofar_issuetracker/issues/5807
# Understand the proble
lofar_add_test(tBeamFormerSubbandProcProcessSb tBeamFormerSubbandProcProcessSb.cc)

lofar_add_test(tCorrelatorSubbandProcProcessSb
  tCorrelatorSubbandProcProcessSb.cc)
lofar_add_test(tCoherentStokesBeamFormerSubbandProcProcessSb
  tCoherentStokesBeamFormerSubbandProcProcessSb.cc ../Kernels/KernelTestHelpers.cc)
lofar_add_test(tFlysEyeBeamFormerSubbandProcProcessSb
  tFlysEyeBeamFormerSubbandProcProcessSb.cc)

# Trick tests that compile kernels that need to be installed into thinking
# that they can find the kernels using the LOFARROOT environment variable.
if(BUILD_TESTING)
  set_tests_properties(
    tBeamFormerSubbandProcProcessSb
    tCorrelatorSubbandProcProcessSb
    tCoherentStokesBeamFormerSubbandProcProcessSb
    tFlysEyeBeamFormerSubbandProcProcessSb
    PROPERTIES ENVIRONMENT "LOFARROOT=${PACKAGE_SOURCE_DIR}"
    )
endif(BUILD_TESTING)
