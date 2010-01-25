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
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_BINARY_DIR}/../src/convertimagerparset
  ${CMAKE_CURRENT_BINARY_DIR}/convertimagerparset)

lofar_add_test(tconvertimagerparset DEPENDS convertimagerparset)
lofar_add_test(tmwimager DEPENDS convertimagerparset)
#lofar_add_test(tmwimager-dd DEPENDS convertimagerparset <AND MORE>)
