//# CommandProcessorSolver.cc: Controls execution of processing steps by a
//# solver process.
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSControl/CommandProcessorSolver.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/SynchronizeCommand.h>
#include <BBSControl/MultiStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/AddStep.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/RefitStep.h>
#include <BBSKernel/Solver.h>

namespace LOFAR
{
namespace BBS
{

// Forces registration with the ObjectFactory.
namespace
{
  InitializeCommand cmd0;
  FinalizeCommand   cmd1;
  NextChunkCommand  cmd2;
}

bool CommandProcessorSolver::hasFinished() const
{
  return itsHasFinished;
}

CommandResult CommandProcessorSolver::visit(const InitializeCommand&)
{
  try
  {
    itsSolver->init(itsProcessGroup);
  }
  catch(const Exception &ex)
  {
    return CommandResult(CommandResult::ERROR, ex.text());
  }

  return CommandResult(CommandResult::OK);
}

CommandResult CommandProcessorSolver::visit(const FinalizeCommand&)
{
  itsHasFinished = true;
  return CommandResult(CommandResult::OK);
}

CommandResult CommandProcessorSolver::visit(const NextChunkCommand &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const RecoverCommand &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const SynchronizeCommand &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const MultiStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const PredictStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const SubtractStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const AddStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const CorrectStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const SolveStep &command)
{
  // Initialize a solve task for each calibration group.
  SolverOptions options;
  options.maxIter = command.maxIter();
  options.epsValue = command.epsValue();
  options.epsDerivative = command.epsDerivative();
  options.colFactor = command.colFactor();
  options.lmFactor = command.lmFactor();
  options.balancedEq = command.balancedEq();
  options.useSVD = command.useSVD();

  try
  {
    itsSolver->run(command.calibrationGroups(), options);
  }
  catch(const Exception &ex)
  {
    return CommandResult(CommandResult::ERROR, ex.text());
  }

  return CommandResult(CommandResult::OK);
}

CommandResult CommandProcessorSolver::visit(const ShiftStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::visit(const RefitStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorSolver::unsupported(const Command &command) const
{
  ostringstream message;
  message << "Received unsupported command: " << command.type();
  return CommandResult(CommandResult::ERROR, message.str());
}

} //# namespace BBS
} //# namespace LOFAR
