# $Id$

include(LofarCTest)

lofar_add_test(FIR_FilterTest FIR_FilterTest.cc)

if(UNITTEST++_FOUND)
endif()

