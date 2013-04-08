# $Id$

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_library(cudasupport
  Package__Version.cc
)

lofar_add_bin_program(versioncudasupport versioncudasupport.cc)
