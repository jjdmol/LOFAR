# List of header files that will be installed.
set(inst_HEADERS
  BaseSim.h
  BGLConnection.h
  Connection.h
  ConnectionPool.h
  CSConnection.h
  DataBlobExtra.h
  DataHolder.h
  DH_BlobStreamable.h
  TH_Ethernet.h
  TH_File.h
  TH_Mem.h
  TH_MPI.h
  TH_Null.h
  TH_ShMem.h
  TH_Socket.h
  TransportHolder.h 
)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})

set(${PROJECT_NAME}_INCLUDE_PATH_SUFFIX ${PROJECT_NAME} 
  CACHE INTERNAL "Include path suffix for package \"${PROJECT_NAME}\"")
