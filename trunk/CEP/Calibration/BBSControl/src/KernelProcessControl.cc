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
#include <Common/Exceptions.h>

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
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/Equator.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/VisExpr.h>

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
          itsState(UNDEFINED)
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
        const VisDimensions &dims = itsMeasurement->getDimensions();
        if(!itsCalSession->registerAsKernel(filesys, path, dims.getGrid())) {
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
                << "command: " << endl << *(command.second));

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

      itsInputColumn = command.getInputColumn();

//      // Create model expression.
//      itsModel.reset(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
//        itsMeasurement->getPhaseCenter()));

      // Initialize the chunk selection.
      if(!command.getStations().empty()) {
        itsChunkSelection.setStations(command.getStations());
      }

      Correlation correlation = command.getCorrelation();
      if(!correlation.type.empty()) {
        itsChunkSelection.setPolarizations(correlation.type);
      }

      if(correlation.selection == "AUTO") {
        itsChunkSelection.setBaselineFilter(VisSelection::AUTO);
      }
      else if(correlation.selection == "CROSS") {
        itsChunkSelection.setBaselineFilter(VisSelection::CROSS);
      }

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
      const VisDimensions &dimsObs = itsMeasurement->getDimensions();
      const pair<double, double> freqRangeObs(dimsObs.getFreqRange());
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
        itsChunk = itsMeasurement->read(itsChunkSelection, itsInputColumn,
          true);
      }
      catch(Exception &ex)
      {
        return CommandResult(CommandResult::ERROR, "Failed to read chunk.");
      }

      // Display information about chunk.
      LOG_INFO_STR("Chunk dimensions: " << endl << itsChunk->getDimensions());

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

      // Parse visibility selection.
      vector<baseline_t> baselines;
      vector<string> products;

      if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command))) {
        return CommandResult(CommandResult::ERROR, "Unable to parse visibility"
          " selection.");
      }

      // Construct model expression.
      Model::Ptr model(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
        itsMeasurement->getPhaseCenter(), itsMeasurement->getReferenceFreq()));

      try {
        model->makeForwardExpr(command.modelConfig(), itsChunk, baselines);
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression.");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
      evaluator.setSelection(baselines, products);
      evaluator.process();

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty()) {
        itsMeasurement->write(itsChunkSelection, itsChunk,
          command.outputColumn(), command.writeFlags());
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const SubtractStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Parse visibility selection.
      vector<baseline_t> baselines;
      vector<string> products;

      if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command))) {
        return CommandResult(CommandResult::ERROR, "Unable to parse visibility"
          " selection.");
      }

      // Construct model expression.
      Model::Ptr model(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
        itsMeasurement->getPhaseCenter(), itsMeasurement->getReferenceFreq()));

      try {
        model->makeForwardExpr(command.modelConfig(), itsChunk, baselines);
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression.");
      }

      // Compute & subtract simulated visibilities.
      Evaluator evaluator(itsChunk, model);
      evaluator.setSelection(baselines, products);
      evaluator.setMode(Evaluator::SUBTRACT);
      evaluator.process();

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty()) {
        itsMeasurement->write(itsChunkSelection, itsChunk,
          command.outputColumn(), command.writeFlags());
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const AddStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Parse visibility selection.
      vector<baseline_t> baselines;
      vector<string> products;

      if(!(parseBaselineSelection(baselines, command)
        && parseProductSelection(products, command))) {
        return CommandResult(CommandResult::ERROR, "Unable to parse visibility"
          " selection.");
      }

      // Construct model expression.
      Model::Ptr model(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
        itsMeasurement->getPhaseCenter(), itsMeasurement->getReferenceFreq()));

      try {
        model->makeForwardExpr(command.modelConfig(), itsChunk, baselines);
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression.");
      }

      // Compute & add simulated visibilities.
      Evaluator evaluator(itsChunk, model);
      evaluator.setSelection(baselines, products);
      evaluator.setMode(Evaluator::ADD);
      evaluator.process();

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty()) {
        itsMeasurement->write(itsChunkSelection, itsChunk,
          command.outputColumn(), command.writeFlags());
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const CorrectStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Parse visibility selection.
      vector<baseline_t> baselines;
      vector<string> products;

      if(!(parseBaselineSelection(baselines, command)
          && parseProductSelection(products, command))) {
        return CommandResult(CommandResult::ERROR, "Unable to parse visibility"
          " selection.");
      }

      // Construct model expression.
      Model::Ptr model(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
        itsMeasurement->getPhaseCenter(), itsMeasurement->getReferenceFreq()));

      try {
        model->makeInverseExpr(command.modelConfig(), itsChunk, baselines);
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to construct the"
            " model expression.");
      }

      // Compute simulated visibilities.
      Evaluator evaluator(itsChunk, model);
      evaluator.setSelection(baselines, products);
      evaluator.process();

      // Optionally write the simulated visibilities.
      if(!command.outputColumn().empty()) {
        itsMeasurement->write(itsChunkSelection, itsChunk,
          command.outputColumn(), command.writeFlags());
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult KernelProcessControl::visit(const SolveStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Parse visibility selection.
      vector<baseline_t> baselines;
      vector<string> products;

      if(!(parseBaselineSelection(baselines, command)
          && parseProductSelection(products, command))) {
        return CommandResult(CommandResult::ERROR, "Unable to parse"
          " visibility selection.");
      }

      // Initialize left and right hand expressions.
      VisExpr::Ptr lhs;
      Model::Ptr rhs;

      try {
        lhs.reset(new VisExpr(itsMeasurement->getInstrument(),
          itsMeasurement->getPhaseCenter(), itsChunk, baselines,
          command.shift(), command.direction(), command.resample(),
          command.flagDensityThreshold()));

        if(command.shift()) {
          rhs.reset(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
            command.direction(), itsMeasurement->getReferenceFreq()));

        } else {
          rhs.reset(new Model(itsMeasurement->getInstrument(), *itsSourceDb,
            itsMeasurement->getPhaseCenter(),
            itsMeasurement->getReferenceFreq()));
        }

        rhs->makeForwardExpr(command.modelConfig(), itsChunk, baselines);
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to initialize model"
          " expressions.");
      }

      // Determine evaluation grid.
      const VisDimensions &obsDims = itsChunk->getDimensions();
      Axis::ShPtr freqAxis(obsDims.getFreqAxis());
      Axis::ShPtr timeAxis(obsDims.getTimeAxis());

      if(command.resample()) {
        freqAxis = freqAxis->compress(command.resampleCellSize().freq);
        timeAxis = timeAxis->compress(command.resampleCellSize().time);
      }

      Grid evalGrid(freqAxis, timeAxis);

      // Determine solution grid.
      CellSize size = command.cellSize();

      if(command.globalSolution()) {
        freqAxis = getCalGroupFreqAxis(command.calibrationGroups());
      } else {
        if(size.freq == 0) {
          freqAxis.reset(new RegularAxis(freqAxis->start(), freqAxis->end(), 1,
            true));
        } else if(size.freq > 1) {
            freqAxis = freqAxis->compress(size.freq);
        }
      }

      if(size.time == 0) {
        timeAxis.reset(new RegularAxis(timeAxis->start(), timeAxis->end(), 1,
          true));
      } else if(size.time > 1) {
        timeAxis = timeAxis->compress(size.time);
      }

      Grid solGrid(freqAxis, timeAxis);

      // Determine the number of cells to process simultaneously.
      unsigned int cellChunkSize = (command.cellChunkSize() == 0 ?
        solGrid[TIME]->size() : command.cellChunkSize());

      try {
        if(command.globalSolution()) {
          GlobalSolveController controller(itsKernelIndex, itsSolver, lhs, rhs);

          controller.init(command.parms(), command.exclParms(), evalGrid,
            solGrid, cellChunkSize, command.propagate());

          controller.run();
        } else {
          LocalSolveController controller(lhs, rhs, command.solverOptions());

          controller.init(command.parms(), command.exclParms(), evalGrid,
            solGrid, cellChunkSize, command.propagate());

          controller.run();
        }
      } catch(Exception &ex) {
        return CommandResult(CommandResult::ERROR, "Unable to initialize or run"
          " run solve step controller.");
      }

      // Store solutions to disk.
      ParmManager::instance().flush();

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

      unsigned int idx = 0, count = groups[0];
      while(idx < groups.size() && kernel >= count) {
        count += groups[idx++];
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

    bool KernelProcessControl::parseBaselineSelection
      (vector<baseline_t> &result, const Step &command) const
    {
        const string &filter = command.correlation().selection;
        if(!filter.empty() && filter != "AUTO" && filter != "CROSS")
        {
            LOG_ERROR_STR("Correlation.Selection should be empty or one of"
              " \"AUTO\", \"CROSS\".");
            return false;
        }

        const vector<string> &station1 = command.baselines().station1;
        const vector<string> &station2 = command.baselines().station2;
        if(station1.size() != station2.size())
        {
            LOG_ERROR("Baselines.Station1 and Baselines.Station2 should have"
              " the same length.");
            return false;
        }

        // Filter available baselines.
        set<baseline_t> selection;

        if(station1.empty())
        {
            // If no station groups are speficied, select all the baselines
            // available in the chunk that match the baseline filter.
            const VisDimensions &dims = itsChunk->getDimensions();
            const vector<baseline_t> &baselines = dims.getBaselines();

            vector<baseline_t>::const_iterator baselIt = baselines.begin();
            vector<baseline_t>::const_iterator baselItEnd = baselines.end();
            while(baselIt != baselItEnd)
            {
                if(filter.empty()
                    || (baselIt->first == baselIt->second && filter == "AUTO")
                    || (baselIt->first != baselIt->second && filter == "CROSS"))
                {
                    selection.insert(*baselIt);
                }
                ++baselIt;
            }
        }
        else
        {
            vector<casa::Regex> stationRegex1(station1.size());
            vector<casa::Regex> stationRegex2(station2.size());

            try
            {
                transform(station1.begin(), station1.end(),
                  stationRegex1.begin(), ptr_fun(casa::Regex::fromPattern));
                transform(station2.begin(), station2.end(),
                  stationRegex2.begin(), ptr_fun(casa::Regex::fromPattern));
            }
            catch(casa::AipsError &ex)
            {
                LOG_ERROR_STR("Error parsing include/exclude pattern"
                  " (exception: " << ex.what() << ")");
                return false;
            }

            for(size_t i = 0; i < stationRegex1.size(); ++i)
            {
                // Find the indices of all the stations of which the name
                // matches the regex specified in the context.
                set<unsigned int> stationGroup1, stationGroup2;

                const Instrument &instrument = itsMeasurement->getInstrument();
                for(size_t j = 0; j < instrument.size(); ++j)
                {
                    casa::String stationName(instrument[j].name());

                    if(stationName.matches(stationRegex1[i]))
                    {
                        stationGroup1.insert(j);
                    }

                    if(stationName.matches(stationRegex2[i]))
                    {
                        stationGroup2.insert(j);
                    }
                }

                // Generate all possible baselines (pairs) from the two groups
                // of station indices. If a baseline is available in the chunk
                // _and_ matches the baseline filter, select it for processing.
                const VisDimensions &dims = itsChunk->getDimensions();

                typedef set<unsigned int>::const_iterator IterType;
                for(IterType it1 = stationGroup1.begin(),
                    endIt1 = stationGroup1.end(); it1 != endIt1; ++it1)
                {
                    for(IterType it2 = stationGroup2.begin(),
                        endIt2 = stationGroup2.end(); it2 != endIt2; ++it2)
                    {
                        if(filter.empty()
                            || (*it1 == *it2 && filter == "AUTO")
                            || (*it1 != *it2 && filter == "CROSS"))
                        {
                            baseline_t baseline(*it1, *it2);

                            if(dims.hasBaseline(baseline))
                            {
                                selection.insert(baseline);
                            }
                        }
                    }
                }
            }
        }

        // Verify that at least one baseline is selected.
        if(selection.empty())
        {
            LOG_ERROR("Baseline selection did not match any baselines in the"
                " observation.");
            return false;
        }

        result.resize(selection.size());
        copy(selection.begin(), selection.end(), result.begin());
        return true;
    }

    bool KernelProcessControl::parseProductSelection(vector<string> &result,
        const Step &command) const
    {
        const VisDimensions &dims = itsMeasurement->getDimensions();
        const vector<string> &available = dims.getPolarizations();

        const vector<string> &products = command.correlation().type;
        if(products.empty())
        {
            result = available;
            return true;
        }

        // Select polarization products by name.
        set<string> selection;
        for(size_t i = 0; i < available.size(); ++i)
        {
            // Check if this polarization needs to be processed.
            for(size_t j = 0; j < products.size(); ++j)
            {
                if(available[i] == products[j])
                {
                    selection.insert(available[i]);
                    break;
                }
            }
        }

        // Verify that at least one polarization is selected.
        if(selection.empty())
        {
            LOG_ERROR("Polarization product selection did not match any"
                " polarization product in the observation.");
            return false;
        }

        // Copy selected polarizations.
        result.resize(selection.size());
        copy(selection.begin(), selection.end(), result.begin());
        return true;
    }

} // namespace BBS
} // namespace LOFAR
