# $Id$

include(LofarPackageVersion)
include(PythonInstall)

set(ms_LIB_SRCS
  Package__Version.cc
  MSCreate.cc
  BaselineSelect.cc
  VdsMaker.cc)

set(ms_PROGRAMS
  versionms
  MSSplit
  makevds
  combinevds
  makems
  msplay
  msselect
  msoverview
)

lofar_add_library(ms ${ms_LIB_SRCS})

foreach(prog ${ms_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${ms_PROGRAMS})

install(PROGRAMS
  mssplit
  makemsdistr
  makemsdistr-part
  movemss
  ls_nostderr
  expandcalps
  expandimageps
  expandtargetps
  DESTINATION bin)

# Python modules.
python_install(
  mstools.py 
  DESTINATION lofar)
