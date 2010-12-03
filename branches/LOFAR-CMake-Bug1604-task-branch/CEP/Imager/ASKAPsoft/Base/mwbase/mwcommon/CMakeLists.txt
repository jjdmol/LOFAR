# $Id$

# Define package and its dependencies
lofar_package(askap-mwcommon DEPENDS askap-askap Common Blob)

# Find third-party packages
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/CompileDefinitions.cmake)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/include/mwcommon)

# Generate package/version sources
include(LofarPackageVersion)

# Generate list of sources for library askap_mwcommon
file(GLOB askap_mwcommon_SOURCES src/*.cc)

# Add library
lofar_add_library(askap_mwcommon
  Package__Version.cc
  ${askap_mwcommon_SOURCES})
