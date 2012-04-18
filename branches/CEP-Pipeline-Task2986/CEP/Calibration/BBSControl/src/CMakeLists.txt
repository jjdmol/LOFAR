# $Id$

include(LofarPackageVersion)

set(SRC_LIST
  Package__Version.cc
  AddStep.cc
  BlobStreamableConnection.cc
  Command.cc
  CommandHandlerReducer.cc
  CommandHandlerEstimator.cc
  CommandResult.cc
  CorrectStep.cc
  SharedEstimator.cc
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

  # TODO: These sources are deprecated and should be removed as soon as
  # downstream modules have been updated.
  set(SRC_LIST ${SRC_LIST} GlobalProcessControl.cc KernelProcessControl.cc SolverProcessControl.cc)
endif(HAVE_PQXX)

lofar_add_library(bbscontrol ${SRC_LIST})

lofar_add_bin_program(versionbbscontrol versionbbscontrol.cc)
lofar_add_bin_program(bbs-reducer bbs-reducer.cc)

if(HAVE_PQXX)
  lofar_add_bin_program(bbs-controller bbs-controller.cc)
  lofar_add_bin_program(bbs-shared-estimator bbs-shared-estimator.cc)

  # TODO: These sources are deprecated and should be removed as soon as
  # downstream modules have been updated.
  lofar_add_bin_program(GlobalControl GlobalControl.cc)
  lofar_add_bin_program(KernelControl KernelControl.cc)
  lofar_add_bin_program(SolverControl SolverControl.cc)
endif(HAVE_PQXX)

install(PROGRAMS
  calibrate-stand-alone
  DESTINATION bin)

if(HAVE_PQXX)
  install(PROGRAMS
    calibrate
    calibrate-part
    DESTINATION bin)
endif(HAVE_PQXX)
