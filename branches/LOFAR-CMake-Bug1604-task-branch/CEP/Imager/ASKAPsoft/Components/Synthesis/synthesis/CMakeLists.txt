# $Id$

# Define package and its dependencies
lofar_package(askap-synthesis DEPENDS Blob Common
  askap-askap askap-askapparallel askap-mwcommon askap-scimath)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa coordinates images lattices measures ms scimath tables)

# Set preprocessor variables
include(${ASKAPsoft_SOURCE_DIR}/CompileDefinitions.cmake)

# These were copied from the SConstruct file for Synthesis/synthesis.
add_definitions(-DASKAP_GRID_WITH_POINTERS)
add_definitions(-fcx-limited-range)

# Generate header file with version information.
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_synthesis.h.in
  ${CMAKE_BINARY_DIR}/include/askap_synthesis.h)

# Add source directory
add_subdirectory(src)
