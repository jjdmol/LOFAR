# $Id$
include(LofarCTest)

# FIR_Filter test
lofar_add_test(FIR_FilterTest FIR_FilterTest.cc)  #Add to the list of tests
   								
					  
if(UNITTEST++_FOUND)
endif()
