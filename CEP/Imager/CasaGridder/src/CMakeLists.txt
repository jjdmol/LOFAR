# $Id$

include(LofarPackageVersion)

lofar_add_library(lofargridder
  Package__Version.cc
  Register.cc
  CasaGridder.cc
  LofarATerm.cc
  LofarFTMachine.cc
)

lofar_add_bin_program(versioncasagridder versioncasagridder.cc)
lofar_add_bin_program(lwimager lwimager.cc)
