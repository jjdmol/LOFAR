# $Id$

# Define package and its dependencies
lofar_package(askap-scimath DEPENDS askap-askap Blob Common)

# Find third-party packages
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa coordinates lattices measures scimath tables)
lofar_find_package(GSL REQUIRED)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fft
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/fft)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fitting
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/fitting)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/utils)

# Generate list of sources for library askap_scimath
file(GLOB askap_scimath_SOURCES 
  src/fft/*.cc 
  src/fitting/*.cc 
  src/utils/*.cc)

# Add library
lofar_add_library(askap_scimath
  Package__Version.cc
  ${askap_scimath_SOURCES})
