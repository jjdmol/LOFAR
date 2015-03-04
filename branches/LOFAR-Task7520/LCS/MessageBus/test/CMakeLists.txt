# $Id$

include(LofarCTest)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageFuncs.sh.in
  ${CMAKE_BINARY_DIR}/bin/MessageFuncs.sh @ONLY)
  
lofar_add_test(tMsgBus tMsgBus.cc)

if(UNITTEST++_FOUND)
  lofar_add_test(tMessage tMessage.cc)
endif()


