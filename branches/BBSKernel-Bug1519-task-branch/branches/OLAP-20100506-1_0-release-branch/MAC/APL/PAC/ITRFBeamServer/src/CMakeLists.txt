# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add the directory where the generated C++ headers are stored to the -I path.
#include_directories(${CMAKE_BINARY_DIR}/include/MAC)

lofar_add_bin_program(versionitrfbeamserver versionitrfbeamserver.cc)
lofar_add_bin_program(iBeamServer BeamServerMain.cc BeamServer.cc Beam.cc DigitalBeam.cc AnalogueBeam.cc AnaBeamMgr.cc J2000Converter.cc CasaConverter.cc)
lofar_add_bin_program(ibeamctl beamctl.cc)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/iBeamServer.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/iBeamServer.conf)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ibeamctl.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/ibeamctl.conf)
  
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/iBeamServer.conf
  ${CMAKE_CURRENT_BINARY_DIR}/ibeamctl.conf
  DESTINATION etc)
