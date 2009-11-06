# $Id$

include(LofarCTest)

# We need to create a symlink to the parmdb_main executable in the current
# binary directory, so that the test program(s) can find it.
get_target_property(_parmdb_main parmdb_main LOCATION_${CMAKE_BUILD_TYPE})
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_parmdb_main}
  ${CMAKE_CURRENT_BINARY_DIR}/parmdb_main)

lofar_add_test(tpyparmdb DEPENDS parmdb_main)
