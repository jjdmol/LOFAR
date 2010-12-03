# $Id$

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/dataaccess
  ${CMAKE_BINARY_DIR}/include/dataaccess)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/deconvolution
  ${CMAKE_BINARY_DIR}/include/deconvolution)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/gridding
  ${CMAKE_BINARY_DIR}/include/gridding)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/imageaccess
  ${CMAKE_BINARY_DIR}/include/imageaccess)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/measurementequation
  ${CMAKE_BINARY_DIR}/include/measurementequation)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel
  ${CMAKE_BINARY_DIR}/include/parallel)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/simulation
  ${CMAKE_BINARY_DIR}/include/simulation)

# Add current directory to the -I path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Generate list of sources for library askap_synthesis
file(GLOB askap_synthesis_SOURCES 
  dataaccess/*.cc
  deconvolution/*.cc
  gridding/*.cc
  imageaccess/*.cc
  measurementequation/*.cc
  parallel/*.cc
  simulation/*.cc)

# Add library
lofar_add_library(askap_synthesis
  Package__Version.cc
  ${askap_synthesis_SOURCES})

# Add program(s)
lofar_add_bin_program(cimager apps/cimager.cc CommandLineParser.cc)
