# $Id$

include(LofarCTest)

# Create symbolic links to scripts used by some of the test programs
get_target_property(_loc getparsetvalue LOCATION)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_loc}
  ${CMAKE_CURRENT_BINARY_DIR}/getparsetvalue)
get_target_property(_loc finddproc LOCATION)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_loc}
  ${CMAKE_CURRENT_BINARY_DIR}/finddproc)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${MWCommon_SOURCE_DIR}/src/startdistproc
  ${CMAKE_CURRENT_BINARY_DIR}/startdistproc)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${MWCommon_SOURCE_DIR}/src/socketrun
  ${CMAKE_CURRENT_BINARY_DIR}/socketrun)

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
#lofar_add_test(tParmFacadeDistr DEPENDS parmdb_main)
lofar_add_test(tparmdb DEPENDS parmdb_main)
lofar_add_test(tmakesourcedb DEPENDS makesourcedb parmdb_main)
lofar_add_test(tsetupparmdb)
lofar_add_test(tsetupsourcedb)

