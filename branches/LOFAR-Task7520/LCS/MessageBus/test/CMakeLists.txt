# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)

if(HAVE_UNITTEST++)
  lofar_add_test(tMessage tMessage.cc)
endif(HAVE_UNITTEST++)

if(HAVE_QPID)
  lofar_add_test(tMsgBus tMsgBus.cc)

  lofar_add_test(tPyMsgBus)
  lofar_add_test(tPyProtocols)
endif(HAVE_QPID)
