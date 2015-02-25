# $Id: CMakeLists.txt 21564 2012-07-12 09:59:30Z schoenmakers $

include(LofarPackageVersion)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

lofar_add_sbin_program(
UpdateBrokenAntennaInfoToPVSS 
UpdateBrokenAntennaInfoToPVSS.cc)
