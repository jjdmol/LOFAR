# $Id$

include(LofarCTest)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

lofar_add_test(tControllerDefines tControllerDefines.cc)
lofar_add_test(tAPLUtilities tAPLUtilities.cc)
lofar_add_test(tbitsetUtil tbitsetUtil.cc)
lofar_add_test(tOutOfBand tOutOfBand.cc)
lofar_add_test(tAntennaMapper tAntennaMapper.cc)
lofar_add_test(tAntennaSet tAntennaSet.cc)
lofar_add_test(tAntennaPos tAntennaPos.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceAllocatorTest.log_prop.in
  ${CMAKE_CURRENT_BINARY_DIR}/ResourceAllocatorTest.log_prop)
