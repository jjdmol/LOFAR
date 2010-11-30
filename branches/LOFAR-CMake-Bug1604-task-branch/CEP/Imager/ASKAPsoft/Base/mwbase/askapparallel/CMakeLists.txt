# $Id$

lofar_package(askapparallel DEPENDS askap Blob mwcommon)

lofar_find_package(Casacore REQUIRED COMPONENTS casa)

# Because the ASKAP sources do not include lofar_config.h, we have to set the
# necessary preprocessor variables here manually.
if(HAVE_AIPSPP)
  add_definitions(-DHAVE_AIPSPP)
endif(HAVE_AIPSPP)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Generate header file with version information. 
# WARNING: the current template header file contains hard-coded information!
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/askap_askapparallel.h.in
  ${CMAKE_BINARY_DIR}/include/askap_askapparallel.h)

lofar_add_library(askap_askapparallel
  src/AskapParallel.cc)
