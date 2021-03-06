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
  ParmDBBlob.cc
  ParmDBCasa.cc
  ParmDBLocker.cc
  ParmDBLog.cc
  Parm.cc
  ParmFacadeRep.cc
  ParmFacadeLocal.cc
  ParmFacadeDistr.cc
  ParmFacade.cc
  PatchInfo.cc
  SourceData.cc
  SourceDB.cc
  SourceDBBlob.cc
  SourceDBCasa.cc
  SourceInfo.cc
)

set(parmdb_PROGRAMS
  versionparmdb
  parmdbm
  parmdbremote
  parmexportcal
  makesourcedb
  mergesourcedb
  showsourcedb
)

lofar_add_library(parmdb ${parmdb_LIB_SRCS})

foreach(prog ${parmdb_PROGRAMS})
  lofar_add_bin_program(${prog} ${prog}.cc)
endforeach(prog ${parmdb_PROGRAMS})

lofar_add_bin_scripts(
  setupparmdb
  setupparmdb-part
  setupsourcedb
  setupsourcedb-part
  startparmdbdistr
  parmdbremote-scr)
