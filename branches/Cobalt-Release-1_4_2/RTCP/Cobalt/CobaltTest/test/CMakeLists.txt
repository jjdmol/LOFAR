# $Id$

include(LofarCTest)

# Create symbolic link to the outputProc executable used by the tests below.
get_target_property(_loc outputProc LOCATION)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_loc}
  ${CMAKE_CURRENT_BINARY_DIR}/outputProc)

foreach(_test tMultiPartTABOutput tManyPartTABOutput)
  lofar_add_test(${_test} ${_test}.cc)
  set_tests_properties(${_test} 
    PROPERTIES ENVIRONMENT "LOFARROOT=${GPUProc_SOURCE_DIR}")
endforeach()

