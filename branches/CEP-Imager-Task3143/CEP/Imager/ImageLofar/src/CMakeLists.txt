# $Id$

include(LofarPackageVersion)

lofar_add_library(lofarimage
  Package__Version.cc
)

install(PROGRAMS
  addImagingInfo
  DESTINATION bin)
