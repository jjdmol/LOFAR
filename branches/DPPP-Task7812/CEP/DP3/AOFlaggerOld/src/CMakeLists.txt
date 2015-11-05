# $Id: CMakeLists.txt 30439 2014-11-19 15:04:34Z dijkema $

include(LofarPackageVersion)

lofar_add_library(aoflaggerold
  Package__Version.cc
  AORFlagger.cc
)

lofar_add_bin_program(versionaoflaggerold versionaoflaggerold.cc)
