# $Id: CMakeLists.txt 30439 2014-11-19 15:04:34Z dijkema $

add_library(testdyndppp
  TestDynStep.cc
)

target_link_libraries (testdyndppp dppp)
