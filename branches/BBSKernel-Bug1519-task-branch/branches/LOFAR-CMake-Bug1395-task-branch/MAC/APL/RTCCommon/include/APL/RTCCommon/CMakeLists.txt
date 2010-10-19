# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  gnuplot_i.h
  MarshallBlitz.h
  Observer.h
  PSAccess.h
  Subject.h
  TestSuite.h
  Timestamp.h
  daemonize.h
  RegisterState.h)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location. 
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/RTCCommon
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/APL/RTCCommon)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/APL/RTCCommon)
