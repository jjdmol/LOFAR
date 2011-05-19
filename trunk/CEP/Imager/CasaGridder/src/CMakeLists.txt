# $Id$

include(LofarPackageVersion)

lofar_add_library(lofargridder
  Package__Version.cc
  LofarATerm.cc
  LofarFTMachine.cc
  LofarImager.cc
)

lofar_add_bin_program(versioncasagridder versioncasagridder.cc)
lofar_add_bin_program(awimager awimager.cc)
