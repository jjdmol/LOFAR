//# CommandHandlerReducer.cc: Controls execution of processing steps on (a part
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
#include <BBSControl/CommandHandlerReducer.h>
#include <BBSControl/GlobalSolveController.h>
#include <BBSControl/Messages.h>
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
#include <BBSKernel/UVWFlagger.h>
#include <BBSKernel/Apply.h>
#include <BBSKernel/Estimate.h>
#include <Common/lofar_iomanip.h>

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

CommandHandlerReducer::CommandHandlerReducer(const ProcessGroup &group,
  const Measurement::Ptr &measurement, const ParmDB &parmDB,
  const SourceDB &sourceDB, const casa::Path &logPath)
  : itsProcessGroup(group),
    itsMeasurement(measurement),
    itsParmDB(parmDB),
    itsSourceDB(sourceDB),
    itsLogPath(logPath),
    itsHasFinished(false),
    itsChunkCount(-1)
{
}

bool CommandHandlerReducer::hasFinished() const
{
  return itsHasFinished;
}

CommandResult CommandHandlerReducer::visit(const InitializeCommand &command)
{
  if(command.useSolver())
  {
    if(itsProcessGroup.nProcesses(ProcessGroup::SHARED_ESTIMATOR) == 0)
    {
      return CommandResult(CommandResult::ERROR, "No shared estimator process"
        " available to connect to.");
    }

    const ProcessId &id = itsProcessGroup.id(ProcessGroup::SHARED_ESTIMATOR, 0);
    const unsigned int port = itsProcessGroup.port(0);
    LOG_DEBUG_STR("Defining connection: bbs-shared-estimator@" << id.hostname
      << ":" << port);

    // BlobStreamableConnection takes a 'port' argument of type string?
    ostringstream tmp;
    tmp << port;
    itsEstimatorConnection = shared_ptr<BlobStreamableConnection>
      (new BlobStreamableConnection(id.hostname, tmp.str(), Socket::TCP));

    if(!itsEstimatorConnection->connect())
    {
      ostringstream oss;
      oss << "Unable to connect to shared estimator process:"
        " bbs-shared-estimator@" << id.hostname << ":" << port;
      return CommandResult(CommandResult::ERROR, oss.str());
    }

    // Make our process ID known to the shared estimator.
    itsEstimatorConnection->sendObject(ProcessIdMsg(ProcessId::id()));
  }

  itsChunkCount = -1;
  itsInputColumn = command.inputColumn();

  // Initialize the chunk selection.
  if(command.correlations().size() > 0)
  {
    return CommandResult(CommandResult::ERROR, "Reading a subset of the"
      " available correlations is not supported yet.");
  }

  itsChunkSelection.setBaselineFilter(command.baselines());
  return CommandResult(CommandResult::OK);
}

CommandResult CommandHandlerReducer::visit(const FinalizeCommand&)
{
  itsHasFinished = true;
  return CommandResult(CommandResult::OK);
}

CommandResult CommandHandlerReducer::visit(const NextChunkCommand &command)
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

  ++itsChunkCount;

  // Display information about chunk.
  LOG_DEBUG_STR("Chunk dimensions: " << endl << itsBuffers["DATA"]->dims());

  return CommandResult(CommandResult::OK);
}

CommandResult CommandHandlerReducer::visit(const RecoverCommand &command)
{
  return unsupported(command);
}

CommandResult CommandHandlerReducer::visit(const SynchronizeCommand &command)
{
  return unsupported(command);
}

CommandResult CommandHandlerReducer::visit(const MultiStep &command)
{
  return unsupported(command);
}

CommandResult CommandHandlerReducer::visit(const PredictStep &command)
{
  return simulate(command, Evaluator::EQUATE);
}

CommandResult CommandHandlerReducer::visit(const SubtractStep &command)
{
  return simulate(command, Evaluator::SUBTRACT);
}

CommandResult CommandHandlerReducer::visit(const AddStep &command)
{
  return simulate(command, Evaluator::ADD);
}

CommandResult CommandHandlerReducer::visit(const CorrectStep &command)
{
  // Buffer to operate on.
  VisBuffer::Ptr buffer(itsBuffers["DATA"]);
  ASSERT(buffer);

  // Load precomputed visibilities if required.
  loadPrecomputedVis(command.modelConfig().sources());

  // Determine selected baselines and correlations.
  BaselineMask blMask = itsMeasurement->asMask(command.baselines());
  CorrelationMask crMask = makeCorrelationMask(command.correlations());

  // Use the new algorithm that also updates the covariance matrix if
  // possible.
  if(buffer->nCorrelations() == 4
    && buffer->correlations()[0] == Correlation::XX
    && buffer->correlations()[1] == Correlation::XY
    && buffer->correlations()[2] == Correlation::YX
    && buffer->correlations()[3] == Correlation::YY)
  {
    StationExprLOFAR::Ptr expr;
    try
    {
      expr = StationExprLOFAR::Ptr(new StationExprLOFAR(itsSourceDB, itsBuffers,
        command.modelConfig(), buffer, true, command.useMMSE(),
        command.sigmaMMSE()));
    }
    catch(Exception &ex)
    {
      return CommandResult(CommandResult::ERROR, "Unable to construct the"
        " model expression [" + ex.message() + "]");
    }

    ASSERT(expr);
    apply(expr, buffer, blMask);
  }
  else
  {
    // Construct model expression.
    MeasurementExprLOFAR::Ptr model;
    try
    {
      model.reset(new MeasurementExprLOFAR(itsSourceDB, itsBuffers,
        command.modelConfig(), buffer, blMask, true, command.useMMSE(),
        command.sigmaMMSE()));
    }
    catch(Exception &ex)
    {
      return CommandResult(CommandResult::ERROR, "Unable to construct the"
        " model expression [" + ex.message() + "]");
    }

    // Correct visibilities.
    ASSERT(model);
    Evaluator evaluator(buffer, model);
    evaluator.setBaselineMask(blMask);
    evaluator.setCorrelationMask(crMask);
    evaluator.process();

    // Dump processing statistics to the log.
    ostringstream oss;
    evaluator.dumpStats(oss);
    LOG_DEBUG(oss.str());
  }

  // Flag NaN's introduced in the output (if any).
  buffer->flagsNaN();

  // Write output if required.
  if(!command.outputColumn().empty())
  {
    itsMeasurement->write(buffer, itsChunkSelection, command.outputColumn(),
      command.writeCovariance(), command.writeFlags(), 0x01);
  }

  return CommandResult(CommandResult::OK, "Ok.");
}

CommandResult CommandHandlerReducer::visit(const SolveStep &command)
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
  loadPrecomputedVis(command.modelConfig().sources());

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
      GlobalSolveController controller(itsProcessGroup.index(), equator,
        itsEstimatorConnection);
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
    casa::Path path(itsLogPath);
    path.append(command.logName());
    ParmDBLog log(path.expandedName(), ParmDBLoglevel(command.logLevel()).get(),
      itsChunkCount == 0);

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

CommandResult CommandHandlerReducer::visit(const ShiftStep &command)
{
  return unsupported(command);
}

CommandResult CommandHandlerReducer::visit(const RefitStep &command)
{
  return unsupported(command);
}

CommandResult CommandHandlerReducer::unsupported(const Command &command) const
{
  ostringstream message;
  message << "Received unsupported command (" << command.type() << ")";
  return CommandResult(CommandResult::ERROR, message.str());
}

CommandResult CommandHandlerReducer::simulate(const SingleStep &command,
  Evaluator::Mode mode)
{
  // Buffer to operate on.
  VisBuffer::Ptr buffer(itsBuffers["DATA"]);
  ASSERT(buffer);

  // Load precomputed visibilities if required.
  loadPrecomputedVis(command.modelConfig().sources());

  // Determine selected baselines and correlations.
  BaselineMask blMask = itsMeasurement->asMask(command.baselines());
  CorrelationMask crMask = makeCorrelationMask(command.correlations());

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
  evaluator.setCorrelationMask(crMask);
  evaluator.setMode(mode);
  evaluator.process();

  // Dump processing statistics to the log.
  ostringstream oss;
  evaluator.dumpStats(oss);
  LOG_DEBUG(oss.str());

  // Flag NaN's introduced in the output (if any).
  buffer->flagsNaN();

  // Write output if required.
  if(!command.outputColumn().empty())
  {
    itsMeasurement->write(buffer, itsChunkSelection, command.outputColumn(),
      command.writeCovariance(), command.writeFlags(), 0x01);
  }

  return CommandResult(CommandResult::OK, "Ok.");
}

CorrelationMask
CommandHandlerReducer::makeCorrelationMask(const vector<string> &selection) const
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

void CommandHandlerReducer::loadPrecomputedVis(const vector<string> &patches)
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
CommandHandlerReducer::getCalGroupFreqAxis(const vector<uint32> &groups) const
{
  // Determine the calibration group to which this kernel process belongs,
  // and find the index of the first and last kernel process in that
  // calibration group.
  unsigned int processIndex = itsProcessGroup.index();
  unsigned int groupIndex = 0, count = groups[0];
  while(processIndex >= count)
  {
    ++groupIndex;
    ASSERT(groupIndex < groups.size());
    count += groups[groupIndex];
  }
  ASSERT(processIndex < count);
  LOG_DEBUG_STR("Calibration group index: " << groupIndex);

  const Interval<double> &rangeFirst = itsProcessGroup.range(count
    - groups[groupIndex], FREQ);
  const Interval<double> &rangeLast = itsProcessGroup.range(count - 1, FREQ);

  double freqStart = rangeFirst.start;
  double freqEnd = rangeLast.end;

  LOG_DEBUG_STR("Calibration group frequency range: [" << setprecision(15)
    << freqStart / 1e6 << "," << freqEnd / 1e6 << "] MHz");

  return Axis::ShPtr(new RegularAxis(freqStart, freqEnd, 1, true));
}

} //# namespace BBS
} //# namespace LOFAR
