# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  AntennaMapper.h
  AntennaPos.h
  AntennaSets.h
  APLCommonExceptions.h
  APL_Defines.h
  APLUtilities.h
  bitsetUtil.tcc
  ControllerDefines.h
  CTState.h
  ControllerInfo.h
  ControllerAdmin.h
  ChildControl.h
  ParentControl.h)

# Add definition of include path suffix definition, because the header files
# reside in a non-standard location. 
set(${PACKAGE_NAME}_INCLUDE_PATH_SUFFIX APL/APLCommon
  CACHE INTERNAL "Include path suffix for package \"${PACKAGE_NAME}\"")

# Create symbolic link to include directory, create directory first, if needed.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/APL)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/APL/APLCommon)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/APL/APLCommon)
