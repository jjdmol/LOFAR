# List of header files that will be installed.
set (inst_HEADERS
  Package__Version.h
  DH_ProcControl.h
  PCCmd.h
  ProcControlComm.h
  ProcControlServer.h
  ProcessControl.h
  ProcCtrlProxy.h
  ProcCtrlCmdLine.h
  ProcCtrlRemote.h
  ACCmain.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})

set(${PROJECT_NAME}_INCLUDE_PATH_SUFFIX ${PROJECT_NAME} 
  CACHE INTERNAL "Include path suffix for package \"${PROJECT_NAME}\"")
