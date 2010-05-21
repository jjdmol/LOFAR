# $Id$

include(LofarPackageVersion)

set(ms_LIB_SRCS
  Package__Version.cc
  MSCreate.cc
  VdsMaker.cc)

set(ms_PROGRAMS
  versionms
  MSSplit
  makevds
  combinevds
  makems
  msplay
  msselect
)

lofar_add_library(ms ${ms_LIB_SRCS})

foreach(prog ${ms_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${ms_PROGRAMS})

install(PROGRAMS
  mssplit
  makemsdistr
  makemsdistr-part
  DESTINATION bin)
