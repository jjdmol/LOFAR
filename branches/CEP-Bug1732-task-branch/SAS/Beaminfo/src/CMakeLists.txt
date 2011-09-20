# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

#set(ms_LIB_SRCS)

set(ms_PROGRAMS
  beaminfo
)

#lofar_add_library(ms ${ms_LIB_SRCS})

foreach(prog ${ms_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${ms_PROGRAMS})

install(PROGRAMS
  beaminfo
  DESTINATION bin)

# Python modules.
#python_install(
#  DESTINATION lofar)
