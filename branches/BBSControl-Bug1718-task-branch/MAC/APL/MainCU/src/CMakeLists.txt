# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/MainCU)

lofar_add_library(maincu Package__Version.cc)
lofar_add_bin_program(versionmaincu versionmaincu.cc)

add_subdirectory(MACScheduler)
add_subdirectory(ObservationControl)
