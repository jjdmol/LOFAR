# $Id$

include(LofarPackageVersion)

lofar_add_library(bbscontrol
  Package__Version.cc
  AddStep.cc
  BlobStreamableConnection.cc
  CalSession.cc
  CalSessionTransactors.cc
  Command.cc
  CommandResult.cc
  CorrectStep.cc
  FinalizeCommand.cc
  GlobalProcessControl.cc
  InitializeCommand.cc
  KernelConnection.cc
  KernelProcessControl.cc
  LocalSolveController.cc
  Messages.cc
  MultiStep.cc
  NextChunkCommand.cc
  PredictStep.cc
  RecoverCommand.cc
  RefitStep.cc
  ShiftStep.cc
  SingleStep.cc
  SolverProcessControl.cc
  SolveStep.cc
  SolveTask.cc
  Step.cc
  Strategy.cc
  StreamUtil.cc
  SubtractStep.cc
  SynchronizeCommand.cc
  Types.cc)

lofar_add_bin_program(versionbbscontrol versionbbscontrol.cc)
lofar_add_bin_program(GlobalControl GlobalControl.cc)
lofar_add_bin_program(KernelControl KernelControl.cc)
lofar_add_bin_program(SolverControl SolverControl.cc)

install(PROGRAMS
  calibrate
  calibrate-part
  DESTINATION bin)
