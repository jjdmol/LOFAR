# $Id$

include(LofarPackageVersion)

lofar_add_library(lofarft
  Package__Version.cc
  LofarATerm.cc
  LofarFTMachine.cc
  LofarImager.cc
)

lofar_add_bin_program(versionlofarft versionlofarft.cc)
lofar_add_bin_program(awimager awimager.cc)
