# List of header files that will be installed.
set (inst_HEADERS
  ACCmd.h
  ACAsyncClient.h
  ACRequest.h
  ACSyncClient.h
  ApplControlClient.h
  ApplControlComm.h
  ApplControl.h
  ApplControlServer.h
  DH_ApplControl.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
