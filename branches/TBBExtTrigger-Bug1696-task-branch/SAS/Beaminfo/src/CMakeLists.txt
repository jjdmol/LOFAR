# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

set(beaminfo_LIB_SRCS
  FailedTileInfo
)

#set(beaminfo_PROGRAMS
#  failedtilesinfo
#  failedtiles2ms
#)

#lofar_add_library(beaminfo ${beaminfo_LIB_SRCS})

lofar_add_bin_program(failedtilesinfo failedtilesinfo.cc)
lofar_add_bin_program(failedtiles2ms failedtiles2ms.cc FailedTileInfo.cc)

#install(PROGRAMS
#  failedtilesinfo
#  failedtiles2ms
#DESTINATION bin)
