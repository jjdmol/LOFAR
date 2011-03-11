# $Id$

# Define package and its dependencies
lofar_package(askap-accessors DEPENDS Common askap-askap askap-scimath)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS casa measures scimath tables)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# Generate header file with version information.
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_accessors.h.in
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askap_accessors.h)

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/dataaccess
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/dataaccess)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/imageaccess
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/imageaccess)

# Generate list of sources for library askap_accessors
file(GLOB askap_accessors_SOURCES
  src/dataaccess/*.cc
  src/imageaccess/*.cc)

# Add library
lofar_add_library(askap_accessors
  Package__Version.cc
  ${askap_accessors_SOURCES})
