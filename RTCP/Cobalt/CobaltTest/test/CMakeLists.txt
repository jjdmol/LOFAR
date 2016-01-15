# $Id$

include(LofarCTest)

# Create symbolic link to the outputProc executable used by the tests below.
lofar_create_target_symlink(
  outputProc
  ${CMAKE_CURRENT_BINARY_DIR}/outputProc)

foreach(_test tMultiPartTABOutput tManyPartTABOutput)
  lofar_add_test(${_test} ${_test}.cc DEPENDS outputProc)
  if(BUILD_TESTING)
    set_tests_properties(${_test} 
      PROPERTIES ENVIRONMENT "LOFARROOT=${GPUProc_SOURCE_DIR}")
  endif(BUILD_TESTING)
endforeach()
