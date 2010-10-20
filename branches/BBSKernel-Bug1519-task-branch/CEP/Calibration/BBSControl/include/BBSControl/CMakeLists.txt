# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES
  AddStep.h
  BlobStreamableConnection.h
  CalSession.h
  CalSessionTransactors.h
  Command.h
  CommandResult.h
  CommandVisitor.h
  CorrectStep.h
  Exceptions.h
  FinalizeCommand.h
  GlobalSolveController.h
  GlobalProcessControl.h
  InitializeCommand.h
  KernelProcessControl.h
  LocalSolveController.h
  Messages.h
  MessageHandlers.h
  MultiStep.h
  NextChunkCommand.h
  PredictStep.h
  RecoverCommand.h
  RefitStep.h
  ShiftStep.h
  SingleStep.h
  SolverProcessControl.h
  SolveStep.h
  SolveTask.h
  Step.h
  Strategy.h
  SubtractStep.h
  SynchronizeCommand.h
  Types.h
  DESTINATION include/${PACKAGE_NAME})
