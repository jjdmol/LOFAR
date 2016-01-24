# $Id$

include(LofarCTest)

# We need to create a symlink to the parmdbm executable in the current
# binary directory, so that the test program(s) can find it.
lofar_create_target_symlink(
  parmdbm
  ${CMAKE_CURRENT_BINARY_DIR}/parmdbm)

lofar_add_test(tpyparmdb DEPENDS parmdbm _parmdb)
