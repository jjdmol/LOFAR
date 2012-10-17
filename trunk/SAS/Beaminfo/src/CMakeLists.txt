# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

lofar_add_bin_program(failedtilesinfo failedtilesinfo.cc)

install(PROGRAMS
  failedtilesinfo
DESTINATION bin)
