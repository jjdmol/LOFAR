# $Id: CMakeLists.txt 30439 2014-11-19 15:04:34Z dijkema $

include(LofarPackageVersion)
include(PythonInstall)

lofar_add_library(pythondppp
  Package__Version.cc
  PythonStep.cc
)

lofar_add_bin_program(versionpythondppp versionpythondppp.cc)
