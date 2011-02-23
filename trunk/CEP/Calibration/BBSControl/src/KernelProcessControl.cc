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

#include <BBSControl/LocalSolveController.h>
#include <BBSControl/GlobalSolveController.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/MeasurementExprLOFAR.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/VisEquator.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/UVWFlagger.h>
#include <BBSKernel/Exceptions.h>

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
        string skyDb = ps->getString("ParmDB.Sky");
        string instrumentDb = ps->getString("ParmDB.Instrument");
        string solverDb=ps->getString("ParmLog", "solver");
        string loggingLevel=ps->getString("ParmLoglevel", "NONE");

        try {
          // Open observation part.
          LOG_INFO_STR("Observation part: " << filesys << " : " << path);
          itsMeasurement.reset(new MeasurementAIPS(path));
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open observation part: " << path);
          return false;
        }

        try {
          // Open sky model parameter database.
          LOG_INFO_STR("Sky model: " << skyDb);
          itsSourceDb.reset(new SourceDB(ParmDBMeta("casa", skyDb)));
          ParmManager::instance().initCategory(SKY, itsSourceDb->getParmDB());
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open sky model parameter database: "
            << skyDb);
          return false;
        }

        try {
          // Open instrument model parameter database.
          LOG_INFO_STR("Instrument model: " << instrumentDb);
          ParmManager::instance().initCategory(INSTRUMENT,
            ParmDB(ParmDBMeta("casa", instrumentDb)));
        }
        catch(Exception &e) {
          LOG_ERROR_STR("Failed to open instrument model parameter database: "
            << instrumentDb);
          return false;
        }

//       if(loggingLevel!="NONE")	// If no parmDBLogging is set, skip the initialization
//       {
//			try {
//				// Open ParmDBLog ParmDB for solver logging
//				LOG_INFO_STR("Solver log table: " << solverDb);
//				LOG_INFO_STR("Solver logging level: " << loggingLevel);
//
//				// Depending on value read from parset file for logging level call constructor
//				// with the corresponding enum value
//				//
//				if(loggingLevel=="PERSOLUTION")
//					itsParmLogger.reset(new ParmDBLog(solverDb, ParmDBLog::PERSOLUTION));
//				if(loggingLevel=="PERSOLUTION_CORRMATRIX")
//					itsParmLogger.reset(new ParmDBLog(solverDb, ParmDBLog::PERSOLUTION_CORRMATRIX));
//				if(loggingLevel=="PERITERATION")
//					itsParmLogger.reset(new ParmDBLog(solverDb, ParmDBLog::PERITERATION));
//				if(loggingLevel=="PERITERATION_CORRMATRIX")
//					itsParmLogger.reset(new ParmDBLog(solverDb, ParmDBLog::PERITERATION_CORRMATRIX));
//			}
//			  catch(Exception &e) {
//				 LOG_ERROR_STR("Failed to open instrument model parameter database: "
//					<< instrumentDb);
//				 return false;
//			  }
//		  }

        string key = ps->getString("BBDB.Key", "default");
        itsCalSession.reset(new CalSession(key,
          ps->getString("BBDB.Name"),
          ps->getString("BBDB.User"),
          ps->getString("BBDB.Password", ""),
          ps->getString("BBDB.Host", "localhost"),
          ps->getString("BBDB.Port", "")));

        // Poll until Control is ready to accept workers.
        while(itsCalSession->getState() == CalSession::WAITING_FOR_CONTROL) {
          sleep(3);
        }

        // Try to register as kernel.
        if(!itsCalSession->registerAsKernel(filesys, path,
          itsMeasurement->grid())) {
          LOG_ERROR("Registration denied.");
          return false;
        }

        LOG_INFO_STR("Registration OK.");
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

      // Deallocate chunk.
      ASSERTSTR(itsChunk.use_count() == 0 || itsChunk.use_count() == 1,
        "Chunk shoud be unique (or uninitialized) by now.");
      itsChunk.reset();

      LOG_DEBUG("Reading chunk...");
      try
      {
        itsChunk = itsMeasurement->read(itsChunkSelection, itsInputColumn);
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Failed to read chunk ["
          + ex.message() + "]");
      }

      // Display information about chunk.
      LOG_INFO_STR("Chunk dimensions: " << endl << itsChunk->dimensions());

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

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;

      try
      {
        model.reset(new MeasurementExprLOFAR(command.modelConfig(),
            *itsSourceDb, itsChunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
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

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(itsChunk, itsChunkSelection,
            command.outputColumn(), command.writeFlags(), 1);
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

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;

      try
      {
        model.reset(new MeasurementExprLOFAR(command.modelConfig(),
            *itsSourceDb, itsChunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
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

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(itsChunk, itsChunkSelection,
          command.outputColumn(), command.writeFlags(), 1);
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

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;

      try
      {
        model.reset(new MeasurementExprLOFAR(command.modelConfig(),
            *itsSourceDb, itsChunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
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

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(itsChunk, itsChunkSelection,
          command.outputColumn(), command.writeFlags(), 1);
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

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // Construct model expression.
      MeasurementExprLOFAR::Ptr model;

      try
      {
        model.reset(new MeasurementExprLOFAR(command.modelConfig(),
            *itsSourceDb, itsChunk, blMask, false));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
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

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty())
      {
        itsMeasurement->write(itsChunk, itsChunkSelection,
          command.outputColumn(), command.writeFlags(), 1);
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

      // Determine selected baselines and correlations.
      BaselineMask blMask = itsMeasurement->asMask(command.baselines());
      CorrelationMask crMask = createCorrelationMask(command.correlations());

      // If a UV interval has been specified, flag all samples that fall outside
      // of this interval.
      if(command.uvFlag())
      {
        UVWFlagger flagger(itsChunk);
        flagger.setBaselineMask(blMask);
        flagger.setFlagMask(2);
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
        model.reset(new MeasurementExprLOFAR(command.modelConfig(),
            *itsSourceDb, itsChunk, blMask));
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
          " model expression [" + ex.message() + "]");
      }

      // Determine evaluation grid.
      Axis::ShPtr freqAxis(itsChunk->grid()[FREQ]);
      Axis::ShPtr timeAxis(itsChunk->grid()[TIME]);
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

      // Initialize equator.
      VisEquator::Ptr equator(new VisEquator(itsChunk, model));
      equator->setBaselineMask(blMask);
      equator->setCorrelationMask(crMask);

      if(equator->isSelectionEmpty())
      {
        LOG_WARN_STR("No measured visibility data available in the current"
          " data selection; solving will proceed without data.");
      }

      try
      {
        if(command.globalSolution())
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
        else
        {
          // Initialize local solver.
          Solver::Ptr solver(new Solver(command.solverOptions()));

          // Initialize controller.
          LocalSolveController controller(equator, solver);
          controller.setSolutionGrid(solGrid);
          controller.setSolvables(command.parms(), command.exclParms());
          controller.setPropagateSolutions(command.propagate());
          controller.setCellChunkSize(cellChunkSize);

//			 // Compute a solution of each cell in the solution grid.
//          if(itsParmLogger != NULL)
//          {
//          	 LOG_DEBUG_STR("controller.run(*itsParmLogger)");
//          	 controller.run(*itsParmLogger);		// run with solver criteria logging into ParmDB
//          }
//          else
          	 controller.run();						// run without ParmDB logging
        }
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Unable to initialize or run"
          " solve step controller [" + ex.message() + "]");
      }

      // Dump processing statistics to the log.
      ostringstream oss;
      equator->dumpStats(oss);
      LOG_DEBUG(oss.str());

      // Flush solutions to disk.
      ParmManager::instance().flush();

      // Clear the flags for all samples that fall outside the specified UV
      // interval.
      if(command.uvFlag())
      {
        itsChunk->flagsAndWithMask(~flag_t(2));
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

      double freqStart = itsCalSession->getGrid(first)[0]->start();
      double freqEnd = itsCalSession->getGrid(last)[0]->end();

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

} // namespace BBS
} // namespace LOFAR
