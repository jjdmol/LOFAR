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
  Mutex.h
  Parset.h
  SubbandMetaData.h
  PrintVector.h
  Queue.h
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
  Thread.h
  TransposedData.h
  InputData.h)

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PROJECT_NAME})

# Install header files.
install(FILES ${inst_HEADERS} DESTINATION include/${PROJECT_NAME})
