# $Id$

include(LofarPackageVersion)

lofar_add_library(lofargridder
  Package__Version.cc Register.cc LofarGridder.cc
  )

lofar_add_bin_program(versionlofargridder versionlofargridder.cc)
