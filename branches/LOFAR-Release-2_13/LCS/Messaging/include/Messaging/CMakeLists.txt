# $Id: CMakeLists.txt 1455 2015-08-18 07:31:18Z loose $

# List of header files that will be installed.
set(inst_HEADERS
  LofarMessage.h
  LofarMessages.h
  DefaultSettings.h
  EventMessage.h
  Exceptions.h
  FromBus.h
  Message.h
  MonitoringMessage.h
  ProgressMessage.h
  ServiceMessage.h
  ToBus.h
)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
