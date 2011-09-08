# $Id$

include(LofarPackageVersion)

lofar_add_library(lofarft
  Package__Version.cc
  FFTCMatrix.cc
  LofarATerm.cc
  LofarConvolutionFunction.cc
  LofarImager.cc
  LofarCFStore.cc
  LofarFTMachine.cc
  LofarWTerm.cc
#  LofarVisibilityResamplerBase.cc
  LofarVisResampler.cc
  LofarCubeSkyEquation.cc
)

lofar_add_bin_program(versionlofarft versionlofarft.cc)
lofar_add_bin_program(awimager awimager.cc)
