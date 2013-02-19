# $Id: CMakeLists.txt 13414 2009-06-16 22:15:37Z loose $

include(LofarCTest)

# Add project's source directory to -I path.
include_directories(${PACKAGE_SOURCE_DIR}/src)

include_directories("/cm/shared/apps/UnitTest++/src")

lofar_add_test(tDelayCompensation tDelayCompensation.cc)
target_link_libraries(tDelayCompensation /cm/shared/apps/UnitTest++/libUnitTest++.a)

