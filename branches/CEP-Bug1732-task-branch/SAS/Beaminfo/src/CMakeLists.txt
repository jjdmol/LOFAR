# $Id: CMakeLists.txt 18394 2011-09-20 12:00:08Z duscha $

include(LofarPackageVersion)
#include(PythonInstall)

#set(beaminfo_LIB_SRCS)

set(beaminfo_PROGRAMS
  addbeaminfo
)

#lofar_add_library(ms ${beaminfo_LIB_SRCS})

foreach(prog ${beaminfo_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${beaminfo_PROGRAMS})

#lofar_add_bin_program(addbeaminfo addbeaminfo.cc)

install(PROGRAMS
#  addbeaminfo
  DESTINATION bin)

# Python modules.
#python_install(
#  DESTINATION lofar)
