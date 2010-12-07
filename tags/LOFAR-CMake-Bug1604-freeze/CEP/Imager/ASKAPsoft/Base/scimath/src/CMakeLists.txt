# $Id$

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/fft
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/fft)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/fitting
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/fitting)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/utils)

# Generate list of sources for library askap_scimath
file(GLOB askap_scimath_SOURCES 
  fft/*.cc 
  fitting/*.cc 
  utils/*.cc)

# Add library
lofar_add_library(askap_scimath
  Package__Version.cc
  ${askap_scimath_SOURCES})
