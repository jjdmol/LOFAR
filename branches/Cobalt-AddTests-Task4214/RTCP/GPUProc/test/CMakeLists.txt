# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tCorrelateRealData.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/tCorrelateRealData.sh
  @ONLY)

lofar_add_test(tCorrelateRealData.sh)

#lofar_add_test(tDelayCompensation tDelayCompensation.cc)
