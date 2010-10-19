# $Id$

include(LofarCTest)

# Create symbolic links to scripts used by the tmakems test program.
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

lofar_add_test(tMSSplit tMSSplit.cc)
lofar_add_test(tcombinevds DEPENDS combinevds)
lofar_add_test(tmakems DEPENDS makems)
