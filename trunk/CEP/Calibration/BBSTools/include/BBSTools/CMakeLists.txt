# $Id: CMakeLists.txt 16068 2011-10-21 16:09:58Z duscha $

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES
   addUV2MS.h
   DESTINATION include/${PACKAGE_NAME})
