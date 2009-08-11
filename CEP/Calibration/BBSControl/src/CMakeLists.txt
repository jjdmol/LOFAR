#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
  GlobalSolveController.cc
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
