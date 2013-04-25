# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $


include(LofarCTest)

# FIR_Filter test
cuda_add_executable(FIR_FilterTest FIR_FilterTest.cu  )  							# Compile the test use the 
target_link_libraries(FIR_FilterTest
                      ${${PACKAGE_NAME}_LINK_LIBRARIES}  							# Also link to the libs of COBALT
					  cuda) 														# link to cuda	
add_test(FIR_FilterTest FIR_FilterTest)												# Add to the list of tests
#file(COPY  ../../src/Kernels/FIR_Filter.cu DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) # Copy the kernel source to the current location( runtime compilation yay!)

if(UNITTEST++_FOUND)
endif()
