# $Id: CMakeLists.txt 18904 2011-09-28 12:15:43Z diepen $

#include(LofarCTest)
ENABLE_TESTING()
ADD_TEST(PipelineTests python ${CMAKE_CURRENT_BINARY_DIR}/CEP/Pipeline/framework/lofarpipe/tests/UnitTester.py -x results.xml)
