//# CommandProcessorCore.cc: Controls execution of processing steps on (a part
//# of) the visibility data.
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
#include <BBSControl/CommandProcessorCore.h>

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

#include <BBSKernel/ParmManager.h>
#include <BBSKernel/MeasurementExprLOFAR.h>
//#include <BBSKernel/ParmManager.h>
//#include <BBSKernel/Evaluator.h>
//#include <BBSKernel/VisEquator.h>
//#include <BBSKernel/Solver.h>
//#include <BBSKernel/Exceptions.h>
#include <BBSControl/GlobalSolveController.h>
#include <BBSKernel/UVWFlagger.h>

#include <BBSKernel/Apply.h>
#include <BBSKernel/Estimate.h>

#include <BBSControl/BlobStreamableConnection.h>
#include <Common/lofar_iomanip.h>

#include <BBSControl/Messages.h>

namespace LOFAR
{
namespace BBS
{

// Forces registration with Object Factory.
namespace
{
  InitializeCommand cmd0;
  FinalizeCommand   cmd1;
  NextChunkCommand  cmd2;
}

CommandProcessorCore::CommandProcessorCore(const ProcessGroup &group,
    const Measurement::Ptr &measurement, const ParmDB &parmDB,
    const SourceDB &sourceDB)
    : itsProcessGroup(group),
      itsMeasurement(measurement),
      itsParmDB(parmDB),
      itsSourceDB(sourceDB),
      itsHasFinished(false)
{
}

bool CommandProcessorCore::hasFinished() const
{
  return itsHasFinished;
}

CommandResult CommandProcessorCore::visit(const InitializeCommand &command)
{
  if(command.useSolver()) {
    ASSERT(itsProcessGroup.getProcessCount(ProcessGroup::SOLVER) == 1);

    const Process solverProcess = itsProcessGroup.process(ProcessGroup::SOLVER,
      0);
    ProcessId solverId = solverProcess.id;
    const size_t port = solverProcess.port;

    LOG_DEBUG_STR("Defining connection: solver@" << solverId.hostname
      << ":" << port);

    // BlobStreamableConnection takes a 'port' argument of type string?
    ostringstream tmp;
    tmp << port;
    itsSolverConnection.reset(new BlobStreamableConnection(solverId.hostname,
      tmp.str(), Socket::TCP));

    if(!itsSolverConnection->connect()) {
      return CommandResult(CommandResult::ERROR, "Unable to connect to"
        " solver.");
    }

    // Make our process id known to the global solver.
    itsSolverConnection->sendObject(ProcessIdMsg(ProcessId::id()));
  }

  itsInputColumn = command.inputColumn();

  // Initialize the chunk selection.
  if(command.correlations().size() > 0)
  {
    return CommandResult(CommandResult::ERROR, "Reading a subset of the"
      " available correlations is not supported yet.");
  }

  itsChunkSelection.setBaselineFilter(command.baselines());
  return CommandResult(CommandResult::OK, "");
}

CommandResult CommandProcessorCore::visit(const FinalizeCommand &command)
{
  itsHasFinished = true;
  return CommandResult(CommandResult::OK, "");
}

CommandResult CommandProcessorCore::visit(const NextChunkCommand &command)
{
  // Check preconditions. Currently it is assumed that each chunk spans the
  // frequency axis of the _entire_ observation.
  const pair<double, double> freqRangeCmd(command.getFreqRange());
  const pair<double, double> freqRangeObs =
    itsMeasurement->grid()[FREQ]->range();

  ASSERT((freqRangeObs.first >= freqRangeCmd.first
    || casa::near(freqRangeObs.first, freqRangeCmd.first))
    && (freqRangeObs.second <= freqRangeCmd.second
    || casa::near(freqRangeObs.second, freqRangeCmd.second)));

  // Update domain.
  const pair<double, double> timeRangeCmd(command.getTimeRange());

  // Notify ParmManager. NB: The domain from the NextChunkCommand is used
  // for parameters. This domain spans the entire observation in frequency
  // (even though locally visibility data is available for only a small part
  // of this domain).
  Box domain(Point(freqRangeCmd.first, timeRangeCmd.first),
    Point(freqRangeCmd.second, timeRangeCmd.second));
  ParmManager::instance().setDomain(domain);

  // Update chunk selection.
  itsChunkSelection.clear(VisSelection::TIME_START);
  itsChunkSelection.clear(VisSelection::TIME_END);
  itsChunkSelection.setTimeRange(timeRangeCmd.first, timeRangeCmd.second);

  // Deallocate buffers.
  itsBuffers.clear();

  // Read chunk.
  LOG_DEBUG_STR("Reading chunk from column: " << itsInputColumn);
  try
  {
    itsBuffers["DATA"] = itsMeasurement->read(itsChunkSelection,
        itsInputColumn);
  }
  catch(Exception &ex)
  {
    return CommandResult(CommandResult::ERROR, "Failed to read chunk ["
      + ex.message() + "]");
  }

  // Display information about chunk.
  LOG_DEBUG_STR("Chunk dimensions: " << endl << itsBuffers["DATA"]->dims());

  return CommandResult(CommandResult::OK, "");
}

CommandResult CommandProcessorCore::visit(const RecoverCommand &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const SynchronizeCommand &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const MultiStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const PredictStep &command)
{
  return simulate(command, Evaluator::EQUATE);
}

CommandResult CommandProcessorCore::visit(const SubtractStep &command)
{
  return simulate(command, Evaluator::SUBTRACT);
}

CommandResult CommandProcessorCore::visit(const AddStep &command)
{
//  return simulate(command, Evaluator::ADD);
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const CorrectStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const SolveStep &command)
{
  if(command.resample())
  {
    LOG_WARN("Resampling support is unavailable in the current"
      " implementation; resampling will NOT be performed!");
  }

  if(command.shift())
  {
    LOG_WARN("Phase shift support is unavailable in the current"
      " implementation; phase shift will NOT be performed!");
  }

  // Buffer to operate on.
  VisBuffer::Ptr buffer(itsBuffers["DATA"]);

  // Load precomputed visibilities if required.
  loadPrecomputedVis(command.modelConfig().getSources());

  // Determine selected baselines and correlations.
  BaselineMask blMask = itsMeasurement->asMask(command.baselines());
  CorrelationMask crMask = makeCorrelationMask(command.correlations());

  // If a UV interval has been specified, flag all samples that fall outside
  // of this interval.
  if(command.uvFlag())
  {
    UVWFlagger flagger(buffer);
    flagger.setBaselineMask(blMask);
    flagger.setFlagMask(flag_t(2));
    flagger.setUVRange(command.uvRange().first, command.uvRange().second);
    flagger.process();

    // Dump processing statistics to the log.
    ostringstream oss;
    flagger.dumpStats(oss);
    LOG_DEBUG(oss.str());
  }

  // Construct model expression.
  MeasurementExprLOFAR::Ptr model;
  try
  {
    model.reset(new MeasurementExprLOFAR(itsSourceDB, itsBuffers,
        command.modelConfig(), buffer, blMask));
  }
  catch(Exception &ex)
  {
    return CommandResult(CommandResult::ERROR, "Unable to construct the"
      " model expression [" + ex.message() + "]");
  }

  // Determine evaluation grid.
  Axis::ShPtr freqAxis(buffer->grid()[FREQ]);
  Axis::ShPtr timeAxis(buffer->grid()[TIME]);
  Grid evalGrid(freqAxis, timeAxis);

  // Determine solution grid.
  CellSize size = command.cellSize();

  if(command.globalSolution())
  {
    freqAxis = getCalGroupFreqAxis(command.calibrationGroups());
  }
  else
  {
    if(size.freq == 0)
    {
      freqAxis.reset(new RegularAxis(freqAxis->start(), freqAxis->end(), 1,
        true));
    }
    else if(size.freq > 1)
    {
      freqAxis = freqAxis->compress(size.freq);
    }
  }

  if(size.time == 0)
  {
    timeAxis.reset(new RegularAxis(timeAxis->start(), timeAxis->end(), 1,
      true));
  }
  else if(size.time > 1)
  {
    timeAxis = timeAxis->compress(size.time);
  }

  Grid solGrid(freqAxis, timeAxis);

  // Determine the number of cells to process simultaneously.
  unsigned int cellChunkSize = (command.cellChunkSize() == 0 ?
    solGrid[TIME]->size() : command.cellChunkSize());

  if(command.globalSolution())
  {
    ASSERTSTR(command.algorithm() == "L2"
        && command.mode() == "COMPLEX"
        && !command.reject(), "Global calibration only supports Solve.Mode"
        " = COMPLEX, Solve.Algorithm = L2, and Solve.OutlierRejection ="
        " F.");

    // Initialize equator.
    VisEquator::Ptr equator(new VisEquator(buffer, model));
    equator->setBaselineMask(blMask);
    equator->setCorrelationMask(crMask);

    if(equator->isSelectionEmpty())
    {
      LOG_WARN_STR("No measured visibility data available in the current"
        " data selection; solving will proceed without data.");
    }

    try
    {
      // Initialize controller.
      GlobalSolveController controller(itsProcessGroup.getIndex(), equator,
        itsSolverConnection);
      controller.setSolutionGrid(solGrid);
      controller.setSolvables(command.parms(), command.exclParms());
      controller.setPropagateSolutions(command.propagate());
      controller.setCellChunkSize(cellChunkSize);

      // Compute a solution of each cell in the solution grid.
      controller.run();
    }
    catch(Exception &ex)
    {
      return CommandResult(CommandResult::ERROR, "Unable to initialize or"
        " run solve step controller [" + ex.message() + "]");
    }

    // Dump processing statistics to the log.
    ostringstream oss;
    equator->dumpStats(oss);
    LOG_DEBUG(oss.str());
  }
  else
  {
    EstimateOptions::Mode mode = EstimateOptions::asMode(command.mode());
    if(!EstimateOptions::isDefined(mode)) {
      return CommandResult(CommandResult::ERROR, "Unsupported mode: "
        + command.mode());
    }

    EstimateOptions::Algorithm algorithm =
      EstimateOptions::asAlgorithm(command.algorithm());
    if(!EstimateOptions::isDefined(algorithm)) {
      return CommandResult(CommandResult::ERROR, "Unsupported algorithm: "
        + command.algorithm());
    }

    if(algorithm == EstimateOptions::L1 && command.epsilon().empty()) {
      return CommandResult(CommandResult::ERROR, "L1 epsilon vector should"
        " not be empty.");
    }

    if(command.reject() && command.rmsThreshold().empty()) {
      return CommandResult(CommandResult::ERROR, "Threshold vector should"
        " not be empty.");
    }

    SolverOptions lsqOptions;
    lsqOptions.maxIter = command.maxIter();
    lsqOptions.epsValue = command.epsValue();
    lsqOptions.epsDerivative = command.epsDerivative();
    lsqOptions.colFactor = command.colFactor();
    lsqOptions.lmFactor = command.lmFactor();
    lsqOptions.balancedEq = command.balancedEq();
    lsqOptions.useSVD = command.useSVD();

    EstimateOptions options(mode, algorithm, command.reject(),
      cellChunkSize, command.propagate(), ~flag_t(0), flag_t(4),
      lsqOptions);
    options.setThreshold(command.rmsThreshold().begin(),
      command.rmsThreshold().end());
    options.setEpsilon(command.epsilon().begin(), command.epsilon().end());

    // Open solution log.
//    casa::Path path(itsPath);
//    path.append(command.logName());
    casa::Path path(command.logName());
//    ParmDBLog log(path.absoluteName(),
//      ParmDBLoglevel(command.logLevel()).get(), itsChunkCount == 0);
    ParmDBLog log(path.absoluteName(),
      ParmDBLoglevel(command.logLevel()).get());

    estimate(log, buffer, blMask, crMask, model, solGrid,
      ParmManager::instance().makeSubset(command.parms(),
      command.exclParms(), model->parms()), options);
  }

  // Flush solutions to disk.
  ParmManager::instance().flush();

  // Clear the flags for all samples that fall outside the specified UV
  // interval.
  if(command.uvFlag())
  {
    buffer->flagsAndWithMask(~flag_t(2));
  }

  return CommandResult(CommandResult::OK, "Ok.");
}

CommandResult CommandProcessorCore::visit(const ShiftStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::visit(const RefitStep &command)
{
  return unsupported(command);
}

CommandResult CommandProcessorCore::unsupported(const Command &command) const
{
  ostringstream message;
  message << "Received unsupported command (" << command.type() << ")";
  return CommandResult(CommandResult::ERROR, message.str());
}

CommandResult CommandProcessorCore::simulate(const SingleStep &command,
  Evaluator::Mode mode)
{
  // Buffer to operate on.
  VisBuffer::Ptr buffer(itsBuffers["DATA"]);
  ASSERT(buffer);

  // Load precomputed visibilities if required.
  loadPrecomputedVis(command.modelConfig().getSources());

  // Determine selected baselines.
  BaselineMask blMask = itsMeasurement->asMask(command.baselines());

  // Construct model expression.
  MeasurementExprLOFAR::Ptr model;
  try
  {
    model.reset(new MeasurementExprLOFAR(itsSourceDB, itsBuffers,
      command.modelConfig(), buffer, blMask));
  }
  catch(Exception &ex)
  {
    return CommandResult(CommandResult::ERROR, "Unable to construct the"
      " model expression [" + ex.message() + "]");
  }

  // Compute simulated visibilities.
  Evaluator evaluator(buffer, model);
  evaluator.setBaselineMask(blMask);
  evaluator.setCorrelationMask(makeCorrelationMask(command.correlations()));
  evaluator.setMode(mode);
  evaluator.process();

  // Dump processing statistics to the log.
  ostringstream oss;
  evaluator.dumpStats(oss);
  LOG_DEBUG(oss.str());

  // Write output if required.
  if(!command.outputColumn().empty())
  {
    itsMeasurement->write(buffer, itsChunkSelection, command.outputColumn(),
      command.writeCovariance(), command.writeFlags(), 0x01);
  }

  return CommandResult(CommandResult::OK, "Ok.");
}

CorrelationMask
CommandProcessorCore::makeCorrelationMask(const vector<string> &selection) const
{
  CorrelationMask mask;

  typedef vector<string>::const_iterator CorrelationIt;
  for(CorrelationIt it = selection.begin(), end = selection.end();
    it != end; ++it)
  {
    try
    {
      mask.set(Correlation::asCorrelation(*it));
    }
    catch(BBSKernelException &ex)
    {
      LOG_WARN_STR("Invalid correlation: " << (*it) << "; will be"
        " ignored.");
    }
  }

  // If no valid correlations specified, select all correlations (default).
  return (mask.empty() ? CorrelationMask(true) : mask);
}

void CommandProcessorCore::loadPrecomputedVis(const vector<string> &patches)
{
  for(vector<string>::const_iterator it = patches.begin(),
      end = patches.end(); it != end; ++it)
  {
    if(it->empty() || (*it)[0] != '@')
    {
      continue;
    }

    BufferMap::const_iterator bufIt = itsBuffers.find(*it);
    if(bufIt == itsBuffers.end())
    {
      itsBuffers[*it] = itsMeasurement->read(itsChunkSelection,
        it->substr(1), false, false);
    }
  }
}

Axis::ShPtr
CommandProcessorCore::getCalGroupFreqAxis(const vector<uint32> &groups) const
{
//  ASSERT(itsKernelIndex >= 0);
//  unsigned int kernel = static_cast<unsigned int>(itsKernelIndex);

  unsigned int kernel = itsProcessGroup.getIndex();
  LOG_DEBUG_STR("My index: " << kernel);

  // Determine the calibration group to which this kernel process belongs,
  // and find the index of the first and last kernel process in that
  // calibration group.
  unsigned int idx = 0, count = groups[0];

  while(kernel >= count)
  {
    ++idx;
    ASSERT(idx < groups.size());
    count += groups[idx];
  }

  ASSERT(kernel < count);
  LOG_DEBUG_STR("Calibration group index: " << idx);

//  ProcessId first = itsCalSession->getWorkerByIndex(CalSession::KERNEL,
//    count - groups[idx]);
//  ProcessId last = itsCalSession->getWorkerByIndex(CalSession::KERNEL,
//    count - 1);

//  double freqStart = itsCalSession->getFreqRange(first).start;
//  double freqEnd = itsCalSession->getFreqRange(last).end;

  const Process &first = itsProcessGroup.process(ProcessGroup::KERNEL,
    count - groups[idx]);
  const Process &last = itsProcessGroup.process(ProcessGroup::KERNEL,
    count - 1);

  double freqStart = first.freqRange.start;
  double freqEnd = last.freqRange.end;

  LOG_DEBUG_STR("Calibration group frequency range: [" << setprecision(15)
    << freqStart / 1e6 << "," << freqEnd / 1e6 << "] MHz");

  return Axis::ShPtr(new RegularAxis(freqStart, freqEnd, 1, true));
}

//void CommandProcessorCore::getParmDBLog(const string &name)
//{
////  casa::Path path(itsPath);
////  path.append(command.logName());
//  map<string, ParmDBLog>::iterator it = itsLogs.find(name);
//  if(it != itsLogs.end()) {
//    return *it;
//  }

//  casa::Path path(name);
//  ParmDBLog &
//  itsLogs[name] = ParmDBLog(path.absoluteName(), ParmDBLoglevel(level).get(),
//      true);
//  }

//  ParmDBLog log(path.absoluteName(),
//    ParmDBLoglevel(command.logLevel()).get(), itsChunkCount == 0);

//}

} //# namespace BBS
} //# namespace LOFAR
