# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add the directory where the generated C++ headers are stored to the -I path.
#include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_library(absdevel
  Package__Version.cc
  Beam.cc
  Beamlet.cc
  Beams.cc
  Beamlets.cc)

lofar_add_bin_program(versionbeam_server versionbeam_server.cc)
lofar_add_bin_program(BeamServer BeamServerMain.cc BeamServer.cc)
lofar_add_bin_program(beamctl beamctl.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/BeamServer.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/BeamServer.conf)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/beamctl.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/beamctl.conf)
  
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/BeamServer.conf
  ${CMAKE_CURRENT_BINARY_DIR}/beamctl.conf
  DESTINATION etc)
