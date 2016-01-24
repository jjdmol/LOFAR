# $Id$

include(LofarCTest)

if(HAVE_QPID)
  lofar_add_test(tObservationStartListener)
endif(HAVE_QPID)
