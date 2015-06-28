# $Id$

include(LofarCTest)

if(HAVE_QPID)
  lofar_add_test(tMessageRouter)
endif(HAVE_QPID)
