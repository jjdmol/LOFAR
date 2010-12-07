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

# Add source directory
add_subdirectory(src)
