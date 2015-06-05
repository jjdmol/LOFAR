# $Id$

include(LofarCTest)

# configure_file(
  # ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  # ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)
  
lofar_add_test(testMCQDaemon)
lofar_add_test(testSCQDaemon)
#lofar_add_test(testNCQLib)
#lofar_add_test(testBrokerConsumercount)
# if(HAVE_QPID)
  # lofar_add_test(tMCQDaemonKillRestart)
# endif(HAVE_QPID)

