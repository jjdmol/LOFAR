# $Id$

include(LofarPackageVersion)

set(SRC_LIST
  Package__Version.cc
  AddStep.cc
  BlobStreamableConnection.cc
  Command.cc
  CommandProcessorCore.cc
  CommandProcessorSolver.cc
  CommandResult.cc
  CorrectStep.cc
  DistributedLMSolver.cc
  FinalizeCommand.cc
  GlobalSolveController.cc
  InitializeCommand.cc
  KernelConnection.cc
  Messages.cc
  MultiStep.cc
  NextChunkCommand.cc
  OptionParser.cc
  PredictStep.cc
  ProcessGroup.cc
  ProcessId.cc
  RecoverCommand.cc
  RefitStep.cc
  ShiftStep.cc
  SingleStep.cc
  SolveStep.cc
  SolveTask.cc
  Step.cc
  Strategy.cc
  StreamUtil.cc
  SubtractStep.cc
  SynchronizeCommand.cc
  Types.cc
  Util.cc)

if(HAVE_PQXX)
  set(SRC_LIST ${SRC_LIST} CalSession.cc CalSessionTransactors.cc)
endif(HAVE_PQXX)

lofar_add_library(bbscontrol ${SRC_LIST})

lofar_add_bin_program(versionbbscontrol versionbbscontrol.cc)
lofar_add_bin_program(bbs-data-processor bbs-data-processor.cc)

if(HAVE_PQXX)
  lofar_add_bin_program(bbs-controller bbs-controller.cc)
  lofar_add_bin_program(bbs-solver bbs-solver.cc)
endif(HAVE_PQXX)

install(PROGRAMS
  calibrate
  calibrate-part
  calibrate-stand-alone
  DESTINATION bin)
