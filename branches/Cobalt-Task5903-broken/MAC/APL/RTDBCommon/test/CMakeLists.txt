# $Id$

include(LofarCTest)

# Extra preprocessor definitions
add_definitions(-DUSES_NEW_IOSTREAMS)

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${PACKAGE_BINARY_DIR}/src)

lofar_add_test(tRTDButilities tRTDButilities.cc)
lofar_add_test(tClaimManager tClaimManager.cc)
