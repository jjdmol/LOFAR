# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add libraries
lofar_add_library(fcnp_cn Package__Version.cc fcnp_cn.cc)
lofar_add_library(fcnp_ion Package__Version.cc fcnp_ion.cc)

# Add executable
lofar_add_executable(versionfcnp versionfcnp.cc)
