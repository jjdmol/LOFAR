# $Id$

lofar_package(synthesis DEPENDS askap askapparallel Blob Common mwcommon scimath synthesis)

lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore REQUIRED COMPONENTS 
  casa coordinates images lattices measures ms scimath tables)
#lofar_find_package(CmdLineParser REQUIRED)
#lofar_find_package(GSL REQUIRED)

# Because the ASKAP sources do not include lofar_config.h, we have to set the
# necessary preprocessor variables here manually.
if(CMAKE_DL_LIBS)
  add_definitions(-DHAVE_DLOPEN)
endif(CMAKE_DL_LIBS)

# This was copied from the SConstruct file for Synthesis/synthesis.
add_definitions(-DASKAP_GRID_WITH_POINTERS)

# Generate header file with version information.
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_synthesis.h.in
  ${CMAKE_BINARY_DIR}/include/askap_synthesis.h)

add_subdirectory(src)
