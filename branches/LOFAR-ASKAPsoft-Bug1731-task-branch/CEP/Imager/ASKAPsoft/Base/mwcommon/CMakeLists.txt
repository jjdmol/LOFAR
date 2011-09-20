# $Id$

# Define package and its dependencies
lofar_package(askap-mwcommon DEPENDS askap-askap Common Blob)

# Find third-party packages
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# Generate header file with version information.
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_mwcommon.h.in
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askap_mwcommon.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mwcommon
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/mwcommon)

# Generate package/version sources
include(LofarPackageVersion)

# Generate list of sources for library askap_mwcommon
file(GLOB askap_mwcommon_SOURCES src/mwcommon/*.cc)

# Add library
lofar_add_library(askap_mwcommon
  Package__Version.cc
  ${askap_mwcommon_SOURCES})
