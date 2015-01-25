# $Id$

include(LofarPackageVersion)

lofar_add_library(testsuite 
  Package__Version.cc
  suite.cpp
  test.cpp)

lofar_add_bin_program(versiontestsuite versiontestsuite.cc)
