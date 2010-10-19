# List of header files that will be installed.
set(inst_HEADERS
  Align.h
  AlignedStdAllocator.h
  Allocator.h
  CN_Command.h
  CN_Configuration.h
  CN_Mapping.h
  CorrelatedData.h
  Config.h
  FilteredData.h
  MultiDimArray.h
  Parset.h
  SubbandMetaData.h
  PrintVector.h
  RSPTimeStamp.h
  SparseSet.h
  Exceptions.h
  StreamableData.h
  PencilBeamData.h
  PencilCoordinates.h
  StokesData.h
  ProcessingPlan.h
  CN_ProcessingPlan.h
  BFRawFormat.h
  TransposedData.h
  TransposedBeamFormedData.h
  InverseFilteredData.h
  InputData.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PACKAGE_NAME})
