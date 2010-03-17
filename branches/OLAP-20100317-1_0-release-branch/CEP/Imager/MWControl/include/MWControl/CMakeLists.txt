# $Id$

# List of header files that will be installed.
set(inst_HEADERS
  MWGlobalSpec.h
  MWLocalSpec.h
  MWMultiSpec.h
  MWParameterHandler.h
  MWSpec.h
  MWStrategySpec.h
  PredifferBBS.h
  PredifferProxy.h
  SolverBBS.h
  SolverProxy.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
