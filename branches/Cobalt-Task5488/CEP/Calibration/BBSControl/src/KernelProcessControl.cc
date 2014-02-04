//#  KernelProcessControl.cc:
//#
//# Copyright (C) 2002-2007
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSControl/KernelProcessControl.h>
#include <BBSControl/Messages.h>
#include <BBSControl/Step.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>

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

#include <Common/ParameterSet.h>
#include <Common/Exception.h>

#include <Blob/BlobStreamable.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>

#include <stdlib.h>
#include <unistd.h>

#include <BBSControl/GlobalSolveController.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/MeasurementExprLOFAR.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/VisEquator.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/UVWFlagger.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Apply.h>
#include <BBSKernel/Estimate.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    // Forces registration with Object Factory.
    namespace
    {
      InitializeCommand cmd0;
      FinalizeCommand   cmd1;
      NextChunkCommand  cmd2;
    }


    //##----   P u b l i c   m e t h o d s   ----##//
    KernelProcessControl::KernelProcessControl()
      :   ProcessControl(),
          itsState(UNDEFINED),
          itsChunkCount(-1),
          itsStepCount(0)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    KernelProcessControl::~KernelProcessControl()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    //##----   PLC interface implementation   ----##//
    tribool KernelProcessControl::define()
    {
      LOG_DEBUG("KernelProcessControl::define()");
      return true;
    }


    tribool KernelProcessControl::init()
    {
      LOG_DEBUG("KernelProcessControl::init()");

      try {
        ParameterSet *ps = globalParameterSet();
        ASSERT(ps);

        string filesys = ps->getString("ObservationPart.Filesystem");
        if(filesys == ".")
        {
            // Work-around for the socketrun script.
            filesys.clear();
        }

        string path = ps->getString("ObservationPart.Path");
        string skyDB = ps->getString("ParmDB.Sky");
        string instrumentDB = ps->getString("ParmDB.Instrument");

        try {
          // Open observation part.
          LOG_INFO_STR("Observation part: " << filesys << " : " << path);
          itsPath = path;
          itsMeasurement.reset(new MeasurementAIPS(path));
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open observation part: " << path);
          return false;
        }

        try {
          // Open sky model parameter database.
          LOG_INFO_STR("Sky model: " << skyDB);
          itsSourceDB.reset(new SourceDB(ParmDBMeta("casa", skyDB)));
          ParmManager::instance().initCategory(SKY, itsSourceDB->getParmDB());
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open sky model parameter database: "
            << skyDB);
          return false;
        }

        try {
          // Open instrument model parameter database.
          LOG_INFO_STR("Instrument model: " << instrumentDB);
          ParmManager::instance().initCategory(INSTRUMENT,
            ParmDB(ParmDBMeta("casa", instrumentDB)));
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open instrument model parameter database: "
            << instrumentDB);
          return false;
        }

        string key = ps->getString("BBDB.Key", "default");
        itsCalSession.reset(new CalSession(key,
          ps->getString("BBDB.Name", (getenv("USER") ? : "")),
          ps->getString("BBDB.User", "postgres"),
          ps->getString("BBDB.Password", ""),
          ps->getString("BBDB.Host", "localhost"),
          ps->getString("BBDB.Port", "")));

        // Poll until Control is ready to accept workers.
        while(itsCalSession->getState() == CalSession::WAITING_FOR_CONTROL) {
          sleep(3);
        }

        // Try to register as kernel.
        if(!itsCalSession->registerAsKernel(filesys, path,
          itsMeasurement->grid()[FREQ], itsMeasurement->grid()[TIME])) {
          LOG_ERROR_STR("Could not register as kernel. There may be stale state"
            " in the database for key: " << key);
          return false;
        }
        LOG_INFO_STR("Registration OK.");

        // Get the global ParameterSet and write it into the HISTORY table.
        itsMeasurement->writeHistory(itsCalSession->getParset());

        setState(RUN);
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      return true;
    }


    tribool KernelProcessControl::run()
    {
      LOG_DEBUG("KernelProcessControl::run()");

      try {
        switch(itsState) {
          default: {
            LOG_ERROR_STR("Unexpected state: " << showState());
            return false;
            break;
          }

          case WAIT: {
            // Wait for a command. Note that this call falls through whenever
            // a new command is inserted.
            if(itsCalSession->waitForCommand())
            {
              setState(RUN);
            }
            break;
          }

          case RUN: {
            pair<CommandId, shared_ptr<Command> > command =
                itsCalSession->getCommand();

            if(command.second) {
              LOG_DEBUG_STR("Executing a " << command.second->type()
                << " command:" << endl << *(command.second));

              // Try to execute the command.
              CommandResult result = command.second->accept(*this);

              // Report the result to the global controller.
              itsCalSession->postResult(command.first, result);

              // If an error occurred, log a descriptive message and exit.
              if(result.is(CommandResult::ERROR)) {
                LOG_ERROR_STR("Error executing " << command.second->type()
                  << " command: " << result.message());
                return false;
              }

              // If the command was a finalize command, log that we are done
              // and exit.
              if(command.second->type() == "Finalize") {
                LOG_INFO("Run completed succesfully.");
                clearRunState();
              }
            }
            else {
              LOG_DEBUG("No queued commands found for this process; will"
                " continue waiting.");
              setState(WAIT);
            }
            break;
          }
        } // switch(itsState)
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      return true;
    }


    tribool KernelProcessControl::pause(const string& /*condition*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::release()
    {
      LOG_INFO("KernelProcessControl::release()");
      LOG_WARN("Not supported");
      /* Here we should properly clean-up; i.e. close open sockets, etc. */
      return indeterminate;
    }


    tribool KernelProcessControl::quit()
    {
      LOG_DEBUG("KernelProcessControl::quit()");
      return true;
    }


    tribool KernelProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::recover(const string& /*source*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::reinit(const string& /*configID*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    string KernelProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return string("");
    }


    //##----   CommandVisitor interface implementation   ----##//
    CommandResult KernelProcessControl::visit(const InitializeCommand &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Reset counters.
      itsChunkCount = -1;
      itsStepCount = 0;

      // Get the index of this kernel process.
      itsKernelIndex = itsCalSession->getIndex();

      // Try to connect to a solver if required.
      // TODO: Retry a couple of times if connect() fails?
      if(command.useSolver()) {
        ProcessId solverId =
          itsCalSession->getWorkerByIndex(CalSession::SOLVER, 0);
        const size_t port = itsCalSession->getPort(solverId);

        LOG_DEBUG_STR("Defining connection: solver@" << solverId.hostname
          << ":" << port);

        // BlobStreamableConnection takes a 'port' argument of type string?
        ostringstream tmp;
        tmp << port;
        itsSolver.reset(new BlobStreamableConnection(solverId.hostname,
          tmp.str(), Socket::TCP));

        if(!itsSolver->connect()) {
          return CommandResult(CommandResult::ERROR, "Unable to connect to"
            " solver.");
        }

        // Make our process id known to the global solver.
        itsSolver->sendObject(ProcessIdMsg(itsCalSession->getProcessId()));
      }

      itsInputColumn = command.inputColumn();

      // Initialize the chunk selection.
      if(command.correlations().size() > 0)
      {
        return CommandResult(CommandResult::ERROR, "Reading a subset of the"
          " available correlations is not supported yet.");
      }

      itsChunkSelection.setBaselineFilter(command.baselines());

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const FinalizeCommand&)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const NextChunkCommand &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Check preconditions. Currently it is assumed that each chunk spans the
      // frequency axis of the _entire_ (meta) measurement.
      const pair<double, double> freqRangeCmd(command.getFreqRange());
      const pair<double, double> freqRangeObs =
        itsMeasurement->grid()[FREQ]->range();

      ASSERT((freqRangeObs.first >= freqRangeCmd.first
        || casa::near(freqRangeObs.first, freqRangeCmd.first))
        && (freqRangeObs.second <= freqRangeCmd.second
        || casa::near(freqRangeObs.second, freqRangeCmd.second)));

      // Update domain.
      const pair<double, double> timeRangeCmd(command.getTimeRange());
      itsDomain = Box(Point(freqRangeCmd.first, timeRangeCmd.first),
        Point(freqRangeCmd.second, timeRangeCmd.second));

      // Notify ParmManager. NB: The domain from the NextChunkCommand is used
      // for parameters. This domain spans the entire observation in frequency
      // (even though locally visibility data is available for only a small part
      // of this domain).
      ParmManager::instance().setDomain(itsDomain);

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
      LOG_INFO_STR("Chunk dimensions: " << endl << itsBuffers["DATA"]->dims());

      // Update counters.
      ++itsChunkCount;
      itsStepCount = 0;

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const RecoverCommand &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return unsupported(command);
    }

    CommandResult KernelProcessControl::visit(const SynchronizeCommand &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return unsupported(command);
    }

    CommandResult KernelProcessControl::visit(const MultiStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return unsupported(command);
    }

    CommandResult KernelProcessControl::visit(const PredictStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Log current chunk and step number.
      LOG_DEBUG_STR("@ chunk " << itsChunkCount << " step " << itsStepCount
        << " type " << command.type() << " name " << command.fullName());
      ++itsStepCount;

      // Buffer to operate on.
      VisBuffer::Ptr chunk(itsBuffers["DATA"]);

      // Load precomputed visibilities if required.
      loadPrecomputedVis(command.modelConfig().sources());

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;
      try
      {
        model.reset(new MeasurementExprLOFAR(*itsSourceDB, itsBuffers,
          command.modelConfig(), chunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(chunk, model);
      evaluator.setBaselineMask(blMask);
      evaluator.setCorrelationMask(crMask);

      if(evaluator.isSelectionEmpty())
      {
        LOG_WARN_STR("No visibility data selected for processing.");
      }

      evaluator.process();

      // Dump processing statistics to the log.
      ostringstream oss;
      evaluator.dumpStats(oss);
      LOG_DEBUG(oss.str());

      // Flag NaN's introduced in the output (if any).
      chunk->flagsNaN();

      // Write output if required.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(chunk, itsChunkSelection, command.outputColumn(),
          command.writeCovariance(), command.writeFlags(), 1);
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const SubtractStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Log current chunk and step number.
      LOG_DEBUG_STR("@ chunk " << itsChunkCount << " step " << itsStepCount
        << " type " << command.type() << " name " << command.fullName());
      ++itsStepCount;

      // Buffer to operate on.
      VisBuffer::Ptr chunk(itsBuffers["DATA"]);

      // Load precomputed visibilities if required.
      loadPrecomputedVis(command.modelConfig().sources());

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;
      try
      {
        model.reset(new MeasurementExprLOFAR(*itsSourceDB, itsBuffers,
          command.modelConfig(), chunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(chunk, model);
      evaluator.setBaselineMask(blMask);
      evaluator.setCorrelationMask(crMask);
      evaluator.setMode(Evaluator::SUBTRACT);

      if(evaluator.isSelectionEmpty())
      {
        LOG_WARN_STR("No visibility data selected for processing.");
      }

      evaluator.process();

      // Dump processing statistics to the log.
      ostringstream oss;
      evaluator.dumpStats(oss);
      LOG_DEBUG(oss.str());

      // Flag NaN's introduced in the output (if any).
      chunk->flagsNaN();

      // Write output if required.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(chunk, itsChunkSelection, command.outputColumn(),
          command.writeCovariance(), command.writeFlags(), 1);
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const AddStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Log current chunk and step number.
      LOG_DEBUG_STR("@ chunk " << itsChunkCount << " step " << itsStepCount
        << " type " << command.type() << " name " << command.fullName());
      ++itsStepCount;

      // Buffer to operate on.
      VisBuffer::Ptr chunk(itsBuffers["DATA"]);

      // Load precomputed visibilities if required.
      loadPrecomputedVis(command.modelConfig().sources());

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;
      try
      {
        model.reset(new MeasurementExprLOFAR(*itsSourceDB, itsBuffers,
          command.modelConfig(), chunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(chunk, model);
      evaluator.setBaselineMask(blMask);
      evaluator.setCorrelationMask(crMask);
      evaluator.setMode(Evaluator::ADD);

      if(evaluator.isSelectionEmpty())
      {
        LOG_WARN_STR("No visibility data selected for processing.");
      }

      evaluator.process();

      // Dump processing statistics to the log.
      ostringstream oss;
      evaluator.dumpStats(oss);
      LOG_DEBUG(oss.str());

      // Flag NaN's introduced in the output (if any).
      chunk->flagsNaN();

      // Write output if required.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(chunk, itsChunkSelection, command.outputColumn(),
          command.writeCovariance(), command.writeFlags(), 1);
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const CorrectStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Log current chunk and step number.
      LOG_DEBUG_STR("@ chunk " << itsChunkCount << " step " << itsStepCount
        << " type " << command.type() << " name " << command.fullName());
      ++itsStepCount;

      // Buffer to operate on.
      VisBuffer::Ptr chunk(itsBuffers["DATA"]);

      // Load precomputed visibilities if required.
      loadPrecomputedVis(command.modelConfig().sources());

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Use the new algorithm that also updates the covariance matrix if
      // possible.
      if(chunk->nCorrelations() == 4
        && chunk->correlations()[0] == Correlation::XX
        && chunk->correlations()[1] == Correlation::XY
        && chunk->correlations()[2] == Correlation::YX
        && chunk->correlations()[3] == Correlation::YY)
      {
        try
        {
          StationExprLOFAR::Ptr expr(new StationExprLOFAR(*itsSourceDB,
            itsBuffers, command.modelConfig(), chunk, true, command.useMMSE(),
            command.sigmaMMSE()));
          apply(expr, chunk, blMask);
        }
        catch(Exception &ex)
        {
          return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression [" + ex.message() + "]");
        }
      }
      else
      {
        // Construct model expression.
        MeasurementExprLOFAR::Ptr model;
        try
        {
          model.reset(new MeasurementExprLOFAR(*itsSourceDB, itsBuffers,
            command.modelConfig(), chunk, blMask, true, command.useMMSE(),
            command.sigmaMMSE()));
        }
        catch(Exception &ex)
        {
          return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression [" + ex.message() + "]");
        }

        // Compute simulated visibilities.
        Evaluator evaluator(chunk, model);
        evaluator.setBaselineMask(blMask);
        evaluator.setCorrelationMask(crMask);

        if(evaluator.isSelectionEmpty())
        {
          LOG_WARN_STR("No visibility data selected for processing.");
        }

        evaluator.process();

        // Dump processing statistics to the log.
        ostringstream oss;
        evaluator.dumpStats(oss);
        LOG_DEBUG(oss.str());
      }

      // Flag NaN's introduced in the output (if any).
      chunk->flagsNaN();

      // Write output if required.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(chunk, itsChunkSelection, command.outputColumn(),
          command.writeCovariance(), command.writeFlags(), 1);
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const SolveStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Log current chunk and step number.
      LOG_DEBUG_STR("@ chunk " << itsChunkCount << " step " << itsStepCount
        << " type " << command.type() << " name " << command.fullName());
      ++itsStepCount;

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
      VisBuffer::Ptr chunk(itsBuffers["DATA"]);

      // Load precomputed visibilities if required.
      loadPrecomputedVis(command.modelConfig().sources());

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // If a UV interval has been specified, flag all samples that fall outside
      // of this interval.
      if(command.uvFlag())
      {
        UVWFlagger flagger(chunk);
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
        model.reset(new MeasurementExprLOFAR(*itsSourceDB, itsBuffers,
            command.modelConfig(), chunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Determine evaluation grid.
      Axis::ShPtr freqAxis(chunk->grid()[FREQ]);
      Axis::ShPtr timeAxis(chunk->grid()[TIME]);
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
        VisEquator::Ptr equator(new VisEquator(chunk, model));
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
          GlobalSolveController controller(itsKernelIndex, equator, itsSolver);
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
        casa::Path path(itsPath);
        path.append(command.logName());
        ParmDBLog log(path.absoluteName(),
          ParmDBLoglevel(command.logLevel()).get(), itsChunkCount == 0);

        estimate(log, chunk, blMask, crMask, model, solGrid,
          ParmManager::instance().makeSubset(command.parms(),
          command.exclParms(), model->parms()), options);
      }

      // Flush solutions to disk.
      ParmManager::instance().flush();

      // Clear the flags for all samples that fall outside the specified UV
      // interval.
      if(command.uvFlag())
      {
        chunk->flagsAndWithMask(~flag_t(2));
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const ShiftStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return unsupported(command);
    }

    CommandResult KernelProcessControl::visit(const RefitStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return unsupported(command);
    }

    //##--------   P r i v a t e   m e t h o d s   --------##//
    CommandResult KernelProcessControl::unsupported(const Command &command)
      const
    {
      ostringstream message;
      message << "Received unsupported command (" << command.type() << ")";
      return CommandResult(CommandResult::ERROR, message.str());
    }

    void KernelProcessControl::setState(State state)
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }

    const string& KernelProcessControl::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_State+1] = {
        "UNDEFINED",
        "WAIT",
        "RUN",
        "<UNKNOWN>"  //# This should ALWAYS be last !!
      };
      if (UNDEFINED < itsState && itsState < N_State) return states[itsState];
      else return states[N_State];
    }

    Axis::ShPtr KernelProcessControl::getCalGroupFreqAxis
        (const vector<uint32> &groups) const
    {
      ASSERT(itsKernelIndex >= 0);
      unsigned int kernel = static_cast<unsigned int>(itsKernelIndex);

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

      ProcessId first = itsCalSession->getWorkerByIndex(CalSession::KERNEL,
        count - groups[idx]);
      ProcessId last = itsCalSession->getWorkerByIndex(CalSession::KERNEL,
        count - 1);

      double freqStart = itsCalSession->getFreqRange(first).start;
      double freqEnd = itsCalSession->getFreqRange(last).end;

      LOG_DEBUG_STR("Calibration group frequency range: [" << setprecision(15)
        << freqStart / 1e6 << "," << freqEnd / 1e6 << "] MHz");

      return Axis::ShPtr(new RegularAxis(freqStart, freqEnd, 1, true));
    }

    CorrelationMask
    KernelProcessControl::createCorrelationMask(const vector<string> &selection)
      const
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

    void KernelProcessControl::loadPrecomputedVis(const vector<string> &patches)
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

} // namespace BBS
} // namespace LOFAR
