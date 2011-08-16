# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

lofar_add_test(tDelayCompensation tDelayCompensation.cc)
