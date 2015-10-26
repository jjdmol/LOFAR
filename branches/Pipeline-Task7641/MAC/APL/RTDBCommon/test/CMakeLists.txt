# $Id$

include(LofarCTest)

# Extra preprocessor definitions
add_definitions(-DUSES_NEW_IOSTREAMS)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/testFuncs.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/testFuncs.sh @ONLY)

lofar_add_test(tRTDButilities tRTDButilities.cc)
lofar_add_test(tClaimManager tClaimManager.cc)

if(BUILD_TESTING)
  set_tests_properties(
    tRTDButilities tClaimManager
    PROPERTIES RUN_SERIAL ON)
endif(BUILD_TESTING)
