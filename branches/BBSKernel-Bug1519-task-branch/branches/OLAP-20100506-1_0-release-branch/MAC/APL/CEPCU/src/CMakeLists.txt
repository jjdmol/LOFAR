# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/CEPCU)

lofar_add_library(cepcu Package__Version.cc)
lofar_add_bin_program(versioncepcu versioncepcu.cc)

add_subdirectory(OnlineControl)
add_subdirectory(PythonControl)
add_subdirectory(CEPlogProcessor)
add_subdirectory(BGPlogProcessor)
#  add_subdirectory(OfflineControl)
