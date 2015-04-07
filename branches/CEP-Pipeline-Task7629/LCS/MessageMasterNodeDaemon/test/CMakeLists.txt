# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)
lofar_add_test(testMCQDaemon)
if(HAVE_QPID)
  #lofar_add_test(testMCQDaemon)
endif(HAVE_QPID)

