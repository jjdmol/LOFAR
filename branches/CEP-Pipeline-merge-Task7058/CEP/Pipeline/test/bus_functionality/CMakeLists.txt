# $Id: CMakeLists.txt 31241 2015-03-20 08:29:38Z mol $

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)

if(HAVE_QPID)

  lofar_add_test(tPyMsgBus)
  lofar_add_test(tPyProtocols)
endif(HAVE_QPID)
