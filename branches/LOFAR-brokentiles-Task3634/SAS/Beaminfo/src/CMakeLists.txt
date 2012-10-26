# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

lofar_add_library(beaminfo
  Package__Version.cc
  FailedTileInfo.cc)

lofar_add_bin_program(failedtilesinfo failedtilesinfo.cc)
lofar_add_bin_program(versionbeaminfo versionbeaminfo.cc)
