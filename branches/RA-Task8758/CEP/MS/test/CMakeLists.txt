# $Id$

include(LofarCTest)

# Create symbolic links to scripts used by the tmakems test script.
lofar_create_target_symlink(
  getparsetvalue 
  ${CMAKE_CURRENT_BINARY_DIR}/getparsetvalue)
lofar_create_target_symlink(
  finddproc 
  ${CMAKE_CURRENT_BINARY_DIR}/finddproc)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${LMWCommon_SOURCE_DIR}/src/startdistproc
  ${CMAKE_CURRENT_BINARY_DIR}/startdistproc)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${LMWCommon_SOURCE_DIR}/src/socketrun
  ${CMAKE_CURRENT_BINARY_DIR}/socketrun)

lofar_add_test(tMSSplit tMSSplit.cc)
lofar_add_test(tBaselineSelect tBaselineSelect.cc)
lofar_add_test(tVdsMaker tVdsMaker.cc DEPENDS combinevds)
lofar_add_test(tcombinevds DEPENDS combinevds)
lofar_add_test(tmakems DEPENDS makems getparsetvalue finddproc)
