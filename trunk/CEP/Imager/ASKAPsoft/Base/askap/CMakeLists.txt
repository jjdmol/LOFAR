# $Id$

# Define package and its dependencies
lofar_package(askap-askap DEPENDS Common)

# Find third-party packages
lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/askap
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askap)

# Generate package/version sources
include(LofarPackageVersion)

# Generate list of sources for library askap_askap
file(GLOB askap_askap_SOURCES src/askap/*.cc)

# Add library
lofar_add_library(askap_askap
  Package__Version.cc
  ${askap_askap_SOURCES})
