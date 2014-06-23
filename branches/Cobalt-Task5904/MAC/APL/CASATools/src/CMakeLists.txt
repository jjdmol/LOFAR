# $Id$

include(LofarPackageVersion)

lofar_add_library(casatools
  Package__Version.cc
  CasaConverter.cc)

lofar_add_bin_program(versioncasatools versioncasatools.cc)

