# List of header files that will be installed.
set (inst_HEADERS
  BlobIO.h
  ConverterClient.h
  ConverterCommand.h
  Converter.h
  ConverterStatus.h
  Coord3D.h
  DH_Request.h
  DH_Result.h
  Direction.h
  Epoch.h
  Exceptions.h
  Position.h
  RequestData.h
  ResultData.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
