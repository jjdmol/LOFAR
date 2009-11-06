# $Id$

include(LofarCTest)

lofar_add_test(tAxis tAxis.cc)
lofar_add_test(tAxisMapping tAxisMapping.cc)
lofar_add_test(tBox tBox.cc)
lofar_add_test(tGrid tGrid.cc)
lofar_add_test(tParmValue tParmValue.cc)
lofar_add_test(tParmDBCasa tParmDBCasa.cc)
lofar_add_test(tParmSet tParmSet.cc)
lofar_add_test(tParmCache tParmCache.cc)
lofar_add_test(tParm tParm.cc)
lofar_add_test(tParmFacade tParmFacade.cc DEPENDS parmdb_main)
lofar_add_test(tSourceDBCasa tSourceDBCasa.cc)
lofar_add_test(tTimeAxis tTimeAxis.cc)
lofar_add_test(tParmFacadeDistr DEPENDS parmdb_main)
lofar_add_test(tparmdb DEPENDS parmdb_main)
lofar_add_test(tmakesourcedb DEPENDS makesourcedb parmdb_main)
lofar_add_test(tsetupparmdb)
lofar_add_test(tsetupsourcedb)

