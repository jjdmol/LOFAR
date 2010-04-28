# $Id$

include(LofarCTest)

# We need to create a symlink to the parmdbm executable in the current
# binary directory, so that the test program(s) can find it.
get_target_property(_parmdbm parmdbm LOCATION_${CMAKE_BUILD_TYPE})
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_parmdbm}
  ${CMAKE_CURRENT_BINARY_DIR}/parmdbm)

lofar_add_test(tpyparmdb DEPENDS parmdbm _parmdb)
