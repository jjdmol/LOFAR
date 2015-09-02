# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)

if(HAVE_UNITTEST++)
  lofar_add_test(tMessage tMessage.cc)
  lofar_add_test(tProtocols tProtocols.cc)
  lofar_add_test(tXMLDoc tXMLDoc.cc)
endif(HAVE_UNITTEST++)

if(HAVE_QPID)
  if(HAVE_UNITTEST++)
    lofar_add_test(tMessageBus tMessageBus.cc)
  endif(HAVE_UNITTEST++)

  lofar_add_test(tPyMsgBus)
  lofar_add_test(tPyProtocols)
endif(HAVE_QPID)
