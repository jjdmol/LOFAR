# $Id$

include(LofarCTest)

# configure_file(
  # ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  # ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)


if(HAVE_QPID)
  lofar_add_test(testCQDaemon)
endif(HAVE_QPID)

