# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $


include(LofarCTest)

# FIR_Filter test

lofar_add_test(FIR_FilterTest FIR_FilterTest.cc)	    								#Add to the list of tests
target_link_libraries(FIR_FilterTest
                      ${${PACKAGE_NAME}_LINK_LIBRARIES}  							# Also link to the libs of COBALT
					  cuda) 		
					  
lofar_add_test(tDelayAndBandPass tDelayAndBandPass.cc)	    								
target_link_libraries(tDelayAndBandPass
                      ${${PACKAGE_NAME}_LINK_LIBRARIES}  							
					  cuda) 
					  
if(UNITTEST++_FOUND)
endif()

