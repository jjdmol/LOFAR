# $Id: CMakeLists.txt 16068 2010-07-23 19:53:58Z zwieten $

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES
    Progress.h
    ProgressTransactors.h
    DESTINATION include/${PACKAGE_NAME})
