# $Id$

# Define package and its dependencies
lofar_package(askap-askapparallel DEPENDS Blob Common 
  askap-askap askap-mwcommon)

# Find third-party packages
lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/askapparallel
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askapparallel)

# Generate header file with version information. 
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_askapparallel.h.in
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askap_askapparallel.h)

# Generate package/version sources
include(LofarPackageVersion)

# Generate list of sources for library askap_askapparallel
file(GLOB askap_askapparallel_SOURCES src/askapparallel/*.cc)

# Add library
lofar_add_library(askap_askapparallel
  Package__Version.cc
  ${askap_askapparallel_SOURCES})
