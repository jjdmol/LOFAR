# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add the directory where the generated C++ headers are stored to the -I path.
#include_directories(${CMAKE_BINARY_DIR}/include/MAC)

# Add the current source directory to the -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(caldevel
  Package__Version.cc
  CalibrationAlgorithm.cc
  CalibrationThread.cc
  DipoleModel.cc
  DipoleModelData.cc
  RemoteStationCalibration.cc
  Source.cc
  SourceData.cc
  SubArrays.cc
  SubArraySubscription.cc)
  
lofar_add_bin_program(versioncal_server versioncal_server.cc)
lofar_add_bin_program(CalServer CalServer.cc ACMProxy.cc)
lofar_add_bin_program(calinfo calinfo.cc)
lofar_add_sbin_program(juggle_acc juggle_acc.cc)

foreach(file CalServer.conf DipoleModel.conf SourceCatalog.conf)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${file}
    DESTINATION etc)
endforeach(file)
