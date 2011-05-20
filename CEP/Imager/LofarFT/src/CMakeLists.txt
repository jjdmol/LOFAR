# $Id$

include(LofarPackageVersion)

lofar_add_library(lofarft
  Package__Version.cc
  LofarATerm.cc
  LofarConvolutionFunction.cc
#  LofarImager.cc
#  LofarFTMachine.cc
  LofarWTerm.cc
  LofarVisibilityResamplerBase.cc
  LofarVisibilityResampler.cc
)

lofar_add_bin_program(versionlofarft versionlofarft.cc)
lofar_add_bin_program(awimager awimager.cc)





