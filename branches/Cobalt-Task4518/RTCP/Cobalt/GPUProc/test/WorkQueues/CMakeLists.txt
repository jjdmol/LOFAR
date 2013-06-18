# $Id$

include(LofarCTest)

if(UNITTEST++_FOUND)
  lofar_add_test(tCorrelatorWorkQueue tCorrelatorWorkQueue.cc)
endif()
