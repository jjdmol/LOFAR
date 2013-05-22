# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/CURTDBDaemons)

lofar_add_library(curtdbdaemons Package__Version.cc)

lofar_add_bin_program(versioncurtdbdaemons versioncurtdbdaemons.cc)

add_subdirectory(LogProcessor)
add_subdirectory(KeyValueLogger)
add_subdirectory(SASGateway)
add_subdirectory(SoftwareMonitor)
