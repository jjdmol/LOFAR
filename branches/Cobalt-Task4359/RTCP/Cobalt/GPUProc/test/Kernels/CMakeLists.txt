# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

lofar_add_test(FIR_FilterTest Kernels/FIR_FilterTest.cc)

if(UNITTEST++_FOUND)
endif()
