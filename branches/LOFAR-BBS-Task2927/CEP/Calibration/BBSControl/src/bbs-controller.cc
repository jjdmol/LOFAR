//# bbs-controller.cc: Application that controls a distributed calibration run.
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
//# $Id:

#include <lofar_config.h>
#include <BBSControl/Package__Version.h>
#include <BBSControl/OptionParser.h>
#include <BBSControl/CalSession.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <LMWCommon/VdsDesc.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>
#include <unistd.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

struct LessKernel
{
  bool operator()(const pair<ProcessId, double> &lhs,
    const pair<ProcessId, double> &rhs)
  {
    return lhs.second < rhs.second;
  }
};

// Compare two axes for equality within a tolerance (using casa::near()).
bool equal(const Axis::ShPtr &lhs, const Axis::ShPtr &rhs)
{
  if(lhs->size() != rhs->size()) {
    return false;
  }

  if(lhs->isRegular() && rhs->isRegular()) {
    return casa::near(lhs->start(), rhs->start())
      && casa::near(lhs->end(), rhs->end());
  }

  for(size_t i = 0, end = lhs->size(); i < end; ++i)
  {
    if(casa::near(lhs->center(i), rhs->center(i))
      && casa::near(lhs->width(i), rhs->width(i))) {
      continue;
    }

    return false;
  }

  return true;
}

Axis::ShPtr getGlobalTimeAxis(const CalSession &session)
{
  vector<ProcessId> kernels =
    session.getWorkersByType(CalSession::KERNEL);
  ASSERT(kernels.size() > 0);

  Axis::ShPtr globalAxis;
  for(size_t i = 0; i < kernels.size(); ++i)
  {
    Axis::ShPtr localAxis = session.getTimeAxis(kernels[i]);
    if(!localAxis) {
      THROW(BBSControlException, "Time axis not known for kernel process: "
        << kernels[i]);
    }

    if(globalAxis && !equal(globalAxis, localAxis)) {
      THROW(CalSessionException, "Time axis inconsistent for kernel"
        " process: " << kernels[i]);
    } else {
      globalAxis = localAxis;
    }
  }

  return globalAxis;
}

void createWorkerIndex(CalSession &session)
{
  vector<ProcessId> kernels =
    session.getWorkersByType(CalSession::KERNEL);
  ASSERT(kernels.size() > 0);

  // Sort kernels processes on start frequency.
  vector<pair<ProcessId, double> > index(kernels.size());
  for(size_t i = 0; i < kernels.size(); ++i)
  {
    index[i] = make_pair(kernels[i],
      session.getFreqRange(kernels[i]).start);
  }

  stable_sort(index.begin(), index.end(), LessKernel());

  // Update the worker register.
  for(size_t i = 0; i < index.size(); ++i)
  {
    session.setWorkerIndex(index[i].first, i);
  }

  vector<ProcessId> solvers =
    session.getWorkersByType(CalSession::SOLVER);

  // Update the worker register.
  for(size_t i = 0; i < solvers.size(); ++i)
  {
    session.setWorkerIndex(solvers[i], i);
  }
}

int main(int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.appendOption("Help", "-h", "--help", "Print usage information"
    " and exit.");
  parser.appendOptionWithDefault("Key", "-k", "--key", "default", "Session"
    " key.");
  parser.appendOptionWithDefault("Name", "-d", "--db-name",
    (getenv("USER") ? : ""), "Name of the database used to store shared"
    " state.");
  parser.appendOptionWithDefault("Host", "-h", "--db-host", "localhost",
    "Hostname of the machine that runs the database server.");
  parser.appendOptionWithDefault("Port", "-p", "--db-port", "5432", "Port on"
    " which the database server is listening.");
  parser.appendOptionWithDefault("User", "-U", "--db-user", "postgres", "User"
    " name for database authentication.");
  parser.appendOptionWithDefault("Password", "-w", "--db-password", "",
    "Password for database authentication.");

  ParameterSet options;
  OptionParser::ArgumentList args = OptionParser::makeArgumentList(argc, argv);

  try
  {
    options = parser.parse(args);
  }
  catch(const OptionParserException &ex)
  {
    cerr << "error: " << ex.text() << endl
      << "Try " << progName << " --help for more information" << endl;
    return 1;
  }

  if(options.getBool("Help", false))
  {
    cout << "Usage: " << progName << " [OPTION]... VDS PARSET" << endl
      << "Calibrate MS blablabla asdasd asdasd asd" << endl << endl
      << "Mandatory arguments to long options are mandatory for short options"
          " too." << endl
      << parser.documentation() << endl << endl
      << "blablabla blabal" << endl;
    return 0;
  }

  if(args.size() != 3)
  {
    cerr << "error: wrong number of arguments." << endl
      << "Try " << progName << " --help for more information" << endl;
    return 1;
  }

  // Read Observation descriptor.
  CEP::VdsDesc vdsDesc(args[1]);

  // Read parameter set.
  ParameterSet parset(args[2]);
  Strategy strategy(parset);

  // Initialize the calibration session.
  string key = options.getString("Key");
  CalSession session(key,
      options.getString("Name"),
      options.getString("User"),
      options.getString("Password"),
      options.getString("Host"),
      options.getString("Port"));

  // Try to become the controller of the session.
  if(!session.registerAsControl()) {
    LOG_ERROR_STR("Could not register as control. There may be stale"
      " state in the database for key: " << key);
    return false;
  }

  // Write the global ParameterSet to the blackboard so that it can then
  // be retrieved by the KernelProcesses and be written to the MS/History
  session.setParset(parset);

  // Initialize the register and switch the session state to allow workers
  // to register.
  session.initWorkerRegister(vdsDesc, strategy.useSolver());
  session.setState(CalSession::WAITING_FOR_WORKERS);

  // Wait for workers to register.
  while(session.slotsAvailable()) {
    sleep(3);
  }

  // All workers have registered. Assign indices sorted on frequency.
  session.setState(CalSession::INITIALIZING);
  createWorkerIndex(session);

  // Determine the frequency range of the observation.
  const ProcessId firstKernel =
    session.getWorkerByIndex(CalSession::KERNEL, 0);
  const ProcessId lastKernel =
    session.getWorkerByIndex(CalSession::KERNEL,
      session.getWorkerCount(CalSession::KERNEL) - 1);
  double itsFreqStart = session.getFreqRange(firstKernel).start;
  double itsFreqEnd = session.getFreqRange(lastKernel).end;

  LOG_INFO_STR("Observation frequency range: [" << itsFreqStart << ","
    << itsFreqEnd << "]");

  // Determine global time axis and verify consistency across all parts.
  Axis::ShPtr itsGlobalTimeAxis = getGlobalTimeAxis(session);
  session.setTimeAxis(itsGlobalTimeAxis);

  // Apply TimeRange selection.
  double itsTimeStart = 0;
  double itsTimeEnd = itsGlobalTimeAxis->size() - 1;

  const vector<string> &window = strategy.timeRange();

  casa::Quantity time;
  if(!window.empty() && casa::MVTime::read(time, window[0])) {
    const pair<size_t, bool> result =
      itsGlobalTimeAxis->find(time.getValue("s"));

    if(result.second) {
      itsTimeStart = result.first;
    }
  }

  if(window.size() > 1 && casa::MVTime::read(time, window[1])) {
    const pair<size_t, bool> result =
      itsGlobalTimeAxis->find(time.getValue("s"), false);

    if(result.first < itsGlobalTimeAxis->size()) {
      itsTimeEnd = result.first;
    }
  }

  double itsChunkStart = itsTimeStart;
  double itsChunkSize = strategy.chunkSize();
  if(itsChunkSize == 0) {
    // If chunk size equals 0, take the whole observation as a single
    // chunk.
    itsChunkSize = itsTimeEnd - itsTimeStart + 1;
  }

  LOG_INFO_STR("Selected time range: [" << itsTimeStart << ","
    << itsTimeEnd << "]");
  LOG_INFO_STR("Chunk size: " << itsChunkSize << " timestamp(s)");

  // Switch session state.
  session.setState(CalSession::PROCESSING);

  // Send InitializeCommand and wait for a reply from all workers.
  const size_t nWorkers = session.getWorkerCount();

  InitializeCommand initCmd(strategy);
  CommandId initId = session.postCommand(initCmd);
  LOG_DEBUG_STR("Initialize command has ID: " << initId);

  // Wait for workers to execute initialize command.
  bool ok = false;
  while(!ok) {
    if(session.waitForResult()) {
      CalSession::CommandStatus status =
        session.getCommandStatus(initId);

      if(status.failed > 0) {
        LOG_ERROR_STR("" << status.failed << " worker(s) failed at"
          " initialization");
        session.setState(CalSession::FAILED);
        return false;
      }

      ok = (status.finished == nWorkers);
    }
  }

  bool finished = false;
  StrategyIterator it;
  while(!finished) {
    if(!it.atEnd()) {
      if((*it)->type() != "Solve") {
        session.postCommand(**it, CalSession::KERNEL);
      } else {
        session.postCommand(**it);
      }

      // Advance to the next command.
      ++it;
    } else if(itsChunkStart <= itsTimeEnd) {
      // NEXT_CHUNK
      ASSERT(itsChunkStart <= itsTimeEnd);
      LOG_DEBUG_STR("itsChunkStart: " << itsChunkStart << " itsTimeEnd: " << itsTimeEnd);

      const double start = itsGlobalTimeAxis->lower(itsChunkStart);
      const double end = itsGlobalTimeAxis->upper(std::min(itsChunkStart
        + itsChunkSize - 1, itsTimeEnd));

      itsChunkStart += itsChunkSize;

      NextChunkCommand cmd(itsFreqStart, itsFreqEnd, start, end);
      CommandId itsWaitId = session.postCommand(cmd, CalSession::KERNEL);
      LOG_DEBUG_STR("Next-chunk command has ID: " << itsWaitId);

      while(true) {
        if(session.waitForResult()) {
          CalSession::CommandStatus status = session.getCommandStatus(itsWaitId);

          if(status.finished > status.failed) {
            it = StrategyIterator(strategy);
            break;
          }
        }
      }
    } else {
      LOG_DEBUG_STR("itsChunkStart: " << itsChunkStart << " itsTimeEnd: " << itsTimeEnd);

      // FINALIZE
      CommandId itsWaitId = session.postCommand(FinalizeCommand());

      while(!finished) {
        if(session.waitForResult()) {
          CalSession::CommandStatus status = session.getCommandStatus(itsWaitId);

          if(status.finished == session.getWorkerCount()) {
            session.setState(status.failed == 0 ? CalSession::DONE : CalSession::FAILED);
            finished = true;
          }
        }
      }
    }
  }

  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}
