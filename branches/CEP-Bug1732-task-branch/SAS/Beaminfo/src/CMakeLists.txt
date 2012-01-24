# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

set(beaminfo_LIB_SRCS
  FailedTileInfo
)

set(beaminfo_PROGRAMS
  addbeaminfo
  failedtiles2ms
)

lofar_add_library(ms ${beaminfo_LIB_SRCS})

foreach(prog ${beaminfo_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${beaminfo_PROGRAMS})

install(PROGRAMS
  addbeaminfo
  failedtiles2ms
DESTINATION bin)

# Python modules.
#python_install(
#  DESTINATION lofar)
