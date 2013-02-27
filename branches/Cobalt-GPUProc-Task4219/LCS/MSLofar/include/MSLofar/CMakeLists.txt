# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# List of header files that will be installed.
set (inst_HEADERS
  MSLofarTable.h
  MSAntennaField.h
  MSAntennaFieldColumns.h
  MSElementFailure.h
  MSElementFailureColumns.h
  MSLofarAntenna.h
  MSLofarAntennaColumns.h
  MSLofarField.h
  MSLofarFieldColumns.h
  MSLofarObservation.h
  MSLofarObsColumns.h
  MSStation.h
  MSStationColumns.h
  BeamTables.h
  FailedTileInfo.h
)

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
