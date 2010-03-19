# $Id$

include(LofarPackageVersion)

set(parmdb_LIB_SRCS
  Package__Version.cc
  Axis.cc
  AxisMapping.cc
  Box.cc
  Grid.cc
  ParmValue.cc
  ParmSet.cc
  ParmMap.cc
  ParmCache.cc
  ParmDBMeta.cc
  ParmDB.cc
  ParmDBCasa.cc
  ParmDBLocker.cc
  Parm.cc
  ParmFacadeRep.cc
  ParmFacadeLocal.cc
  ParmFacadeDistr.cc
  ParmFacade.cc
  SourceDB.cc
  SourceDBCasa.cc)

set(parmdb_PROGRAMS
  versionparmdb
  parmdbm
  parmdbremote
  makesourcedb)

lofar_add_library(parmdb ${parmdb_LIB_SRCS})

foreach(prog ${parmdb_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${parmdb_PROGRAMS})
lofar_add_bin_program(parmdb_main parmdbm.cc)

install(PROGRAMS
  setupparmdb
  setupparmdb-part
  setupsourcedb
  setupsourcedb-part
  startparmdbdistr
  parmdbremote-scr
  DESTINATION bin)
