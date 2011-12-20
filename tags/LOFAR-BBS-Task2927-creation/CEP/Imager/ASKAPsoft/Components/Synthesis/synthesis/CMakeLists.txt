# $Id$

# Define package and its dependencies
lofar_package(askap-synthesis DEPENDS Blob Common
  askap-askap askap-mwcommon askap-scimath askap-accessors)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa coordinates images lattices measures ms scimath tables)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/ASKAPCommon.cmake)

# These were copied from the SConstruct file for Synthesis/synthesis.
add_definitions(-DASKAP_GRID_WITH_POINTERS)
add_definitions(-fcx-limited-range)

# Generate header file with version information.
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_synthesis.h.in
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/askap_synthesis.h)

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/deconvolution
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/deconvolution)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gridding
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/gridding)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/measurementequation
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/measurementequation)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parallel
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/parallel)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation
  ${CMAKE_BINARY_DIR}/include/ASKAPsoft/simulation)

# Add current directory to the -I path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Generate list of sources for library askap_synthesis
file(GLOB askap_synthesis_SOURCES 
  src/deconvolution/*.cc
  src/gridding/*.cc
  src/measurementequation/*.cc
  src/parallel/*.cc
  src/simulation/*.cc)

# Add library
lofar_add_library(askap_synthesis
  Package__Version.cc
  ${askap_synthesis_SOURCES})

# Add program(s)
lofar_add_bin_program(cimager src/apps/cimager.cc CommandLineParser.cc)
