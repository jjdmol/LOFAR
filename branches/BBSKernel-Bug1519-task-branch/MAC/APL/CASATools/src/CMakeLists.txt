# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

include(LofarPackageVersion)

lofar_add_library(casatools
  Package__Version.cc
  CasaConverter.cc)

lofar_add_bin_program(versioncasatools versioncasatools.cc)

