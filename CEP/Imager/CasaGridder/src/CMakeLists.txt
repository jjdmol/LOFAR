# $Id$

include(LofarPackageVersion)

lofar_add_library(lofargridder
  Package__Version.cc Register.cc CasaGridder.cc
  )

lofar_add_bin_program(versioncasagridder versioncasagridder.cc)
