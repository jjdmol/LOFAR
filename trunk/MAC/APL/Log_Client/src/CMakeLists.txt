# $Id$
include(LofarPackageVersion)

# Create symbolic link in binary include directory to current directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/Log_Client)

lofar_add_library(log_client Package__Version.cc)

lofar_add_bin_program(versionlog_client versionlog_client.cc)

lofar_add_bin_program(LogClient LogClientMain.cc LogClient.cc)
install(FILES LogClient.conf DESTINATION etc)
