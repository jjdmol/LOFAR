# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/setlofarroot.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/setlofarroot.sh @ONLY
)

# Create symbolic link to the outputProc executable used by the tests below.
get_target_property(_loc outputProc LOCATION)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${_loc}
  ${CMAKE_CURRENT_BINARY_DIR}/outputProc)

lofar_add_test(tMultiPartTABOutput tMultiPartTABOutput.cc)
lofar_add_test(tManyPartTABOutput tManyPartTABOutput.cc)
