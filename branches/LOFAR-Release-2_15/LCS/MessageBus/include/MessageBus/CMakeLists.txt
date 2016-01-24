# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES
  Exceptions.h
  FromBus.h
  Message.h
  MessageBus.h
  NoQpidFallback.h
  ToBus.h
  XMLDoc.h
  DESTINATION include/${PACKAGE_NAME})

add_subdirectory(Protocols)
