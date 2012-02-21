# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(SRC_LIST
  AddStep.h
  BlobStreamableConnection.h
  Command.h
  CommandProcessorCore.h
  CommandProcessorSolver.h
  CommandResult.h
  CommandVisitor.h
  CorrectStep.h
  DistributedLMSolver.h
  Exceptions.h
  FinalizeCommand.h
  GlobalSolveController.h
  InitializeCommand.h
  Messages.h
  MessageHandlers.h
  MultiStep.h
  NextChunkCommand.h
  OptionParser.h
  PredictStep.h
  ProcessGroup.h
  ProcessId.h
  RecoverCommand.h
  RefitStep.h
  ShiftStep.h
  SingleStep.h
  SolveStep.h
  SolveTask.h
  Step.h
  Strategy.h
  SubtractStep.h
  SynchronizeCommand.h
  Types.h
  Util.h)

if(HAVE_PQXX)
  set(SRC_LIST ${SRC_LIST} CalSession.h CalSessionTransactors.h)
endif(HAVE_PQXX)

# Install header files.
install(FILES ${SRC_LIST} DESTINATION include/${PACKAGE_NAME})
