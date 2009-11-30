# $Id$

include(LofarPackageVersion)

lofar_add_library(mwimager
  Package__Version.cc
  ConvertParset.cc)

lofar_add_bin_program(versionmwimager versionmwimager.cc)
lofar_add_bin_program(convertimagerparset convertimagerparset.cc)
lofar_add_bin_program(combineFacets combineFacets.cc)

install(PROGRAMS
  mwimager
  mwimager-part
  mwimager-askap
  mwimager-casa
  mwimager-dd
  makeCombinedImage-part
  DESTINATION bin)
