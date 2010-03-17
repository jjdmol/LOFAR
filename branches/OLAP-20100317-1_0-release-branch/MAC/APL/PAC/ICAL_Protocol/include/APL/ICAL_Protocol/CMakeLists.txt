# $Id: CMakeLists.txt 14860 2010-01-22 11:12:13Z loose $

# List of header files that will be installed.
set(inst_HEADERS
  AntennaArray.h
  AntennaArrayData.h
  AntennaGains.h
  SharedResource.h
  SpectralWindow.h
  SubArray.h
  CalibrationInterface.h
  ACC.h)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location. 
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/ICAL_Protocol
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link to include directory, create directory first, if needed.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/APL)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/APL/ICAL_Protocol)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/APL/ICAL_Protocol)
