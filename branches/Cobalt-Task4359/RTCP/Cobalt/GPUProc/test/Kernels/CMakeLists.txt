# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $


include(LofarCTest)

cuda_add_executable(FIR_FilterTest FIR_FilterTest.cu ${PACKAGE_SOURCE_DIR}/src/Kernels/FIR_Filter.cu)
target_link_libraries(FIR_FilterTest ${${PACKAGE_NAME}_LINK_LIBRARIES}) #Also link to the libs of COBALT
add_test(FIR_FilterTest FIR_FilterTest)
if(UNITTEST++_FOUND)
endif()
