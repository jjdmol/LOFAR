# $Id$

include(LofarPackageVersion)

lofar_add_library(lofarft
  Package__Version.cc
  FFTCMatrix.cc
  LofarATerm.cc
  LofarATermOld.cc
  LofarConvolutionFunction.cc
  LofarConvolutionFunctionOld.cc
  LofarImager.cc
  LofarCFStore.cc
  LofarFTMachine.cc
  LofarFTMachineOld.cc
  LofarWTerm.cc
#  LofarVisibilityResamplerBase.cc
  LofarVisResampler.cc
  LofarVisResamplerOld.cc
  LofarCubeSkyEquation.cc
)

lofar_add_bin_program(versionlofarft versionlofarft.cc)
lofar_add_bin_program(awimager awimager.cc)
lofar_add_bin_program(makefftwisdom2d makefftwisdom2d.cc)
