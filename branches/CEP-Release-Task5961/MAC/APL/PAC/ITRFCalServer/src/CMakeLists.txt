# $Id$

include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Add the directory where the generated C++ headers are stored to the -I path.
include_directories(${CMAKE_BINARY_DIR}/include/MAC)

# Add the current source directory to the -I path.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_library(caldevel
  Package__Version.cc
  ACC.cc
  ACCcache.cc
  RequestPool.cc
  CalibrationThread.cc
  SubArrayMgr.cc)
  
lofar_add_bin_program(versionitrfcalserver versionitrfcalserver.cc)
lofar_add_bin_program(calinfo calinfo.cc)
lofar_add_bin_program(iCalServer
  CalServerMain.cc CalServer.cc ACMProxy.cc LBACalibration.cc)

foreach(file iCalServer.conf SourceCatalog.conf)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${file})
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${file}
    DESTINATION etc)
endforeach(file)
