# $Id$

include(LofarPackageVersion)

lofar_add_library(applcommon
  Package__Version.cc
  Observation.cc
  StationConfig.cc
  StationInfo.cc)

lofar_add_bin_program(versionapplcommon versionapplcommon.cc)

