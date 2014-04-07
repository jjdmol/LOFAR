# $Id: CMakeLists.txt 14273 2009-10-16 10:08:29Z loose $

# List of header files that will be installed.
set (inst_HEADERS
  Condition.h
  Mutex.h
  Queue.h
  Semaphore.h
  Thread.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})

include(LofarPackageVersion)

lofar_add_library(thread
  Package__Version.cc)

lofar_add_bin_program(versionthread versionthread.cc)
