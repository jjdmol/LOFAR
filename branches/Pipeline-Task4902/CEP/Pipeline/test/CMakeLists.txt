# $Id: CMakeLists.txt 18904 2011-09-28 12:15:43Z diepen $
include(LofarCTest)

add_subdirectory(functional_tests)

lofar_add_test(pipeline_test)


