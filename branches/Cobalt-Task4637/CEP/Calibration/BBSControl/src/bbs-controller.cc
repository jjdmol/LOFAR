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
#include <BBSControl/Util.h>
#include <LMWCommon/VdsDesc.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <casa/BasicMath/Math.h>
#include <unistd.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using LOFAR::operator<<;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

// Compare two axes for equality within tolerance (using casa::near()).
bool equal(const Axis::ShPtr &lhs, const Axis::ShPtr &rhs);

// Try to combine the time axes of all the reducer processes into a single
// global time axis.
Axis::ShPtr getGlobalTimeAxis(const CalSession &session);

// Assign all the worker processes an index. Reducer processes are numbered in
// order of increasing start frequency, staring from 0. Shared estimator
// processes are numbered in order of registration time, starting from 0.
void createWorkerIndex(CalSession &session);

int run(const ParameterSet &options, const OptionParser::ArgumentList &args);

// Application entry point.
int main(int argc, char *argv[])
{
  const string progName = LOFAR::basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.addOption("Help", "-h", "--help", "Print usage information and exit.");
  parser.addOptionWithDefault("Key", "-k", "--key", "default", "Session key.");
  parser.addOptionWithDefault("Name", "-d", "--db-name",
    (getenv("USER") ? : ""), "Name of the database used to store shared"
    " state.");
  parser.addOptionWithDefault("Host", "-H", "--db-host", "localhost", "Hostname"
    " of the machine that runs the database server.");
  parser.addOptionWithDefault("Port", "-p", "--db-port", "5432", "Port on which"
    " the database server is listening.");
  parser.addOptionWithDefault("User", "-U", "--db-user", "postgres", "Username"
    " used for authentication.");
  parser.addOptionWithDefault("Password", "-w", "--db-password", "", "Password"
    " used for authentication.");

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
      << endl
      << "Control the distributed execution of the reduction of the observation"
      " described" << endl
      << "by the specified VDS file. (The VDS file is mainly just a list of"
      " paths to" << endl
      << "measurement sets (MS) that together constitute the observation.) The"
      " reduction" << endl
      << "is described by the PARSET." << endl << endl
      << "Mandatory arguments to long options are mandatory for short options"
          " too." << endl
      << parser.documentation() << endl;
    return 0;
  }

  if(args.size() != 3)
  {
    cerr << "error: wrong number of arguments." << endl
      << "Try " << progName << " --help for more information" << endl;
    return 1;
  }

  try
  {
    int status = run(options, args);
    if(status != 0)
    {
      LOG_ERROR_STR(progName << " terminated due to an error.");
      return status;
    }
  }
  catch(Exception &ex)
  {
    LOG_FATAL_STR(progName << " terminated due to an exception: " << ex);
    return 1;
  }

  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}

int run(const ParameterSet &options, const OptionParser::ArgumentList &args)
{
  // Read observation descriptor.
  CEP::VdsDesc vdsDesc(args[1]);

  // Read parameter set.
  ParameterSet parset(args[2]);
  Strategy strategy(parset);

  // Initialize the calibration session.
  string key = options.getString("Key");
  CalSession session(key, options.getString("Name"), options.getString("User"),
    options.getString("Password"), options.getString("Host"),
    options.getString("Port"));

  // Try to become the controller of the session.
  LOG_INFO_STR("Trying to register as session controller...");
  if(!session.registerAsControl())
  {
    LOG_ERROR_STR("Unable to register. There may be stale state in the database"
      " for the session with session key: " << key);
    return 1;
  }
  LOG_INFO_STR("Registration OK.");

  // Write the global ParameterSet to the shared session state so that it can
  // then be retrieved by the reducer processes and be written to the processing
  // history.
  session.setParset(parset);

  // Initialize the register and switch the session state to allow workers to
  // register.
  session.initWorkerRegister(vdsDesc, strategy.useSolver());
  session.setState(CalSession::WAITING_FOR_WORKERS);

  // Wait for workers to register.
  LOG_INFO_STR("Waiting for workers to register...");
  while(session.slotsAvailable())
  {
    sleep(5);
  }
  LOG_INFO_STR("All workers have registered.");

  // All workers have registered. Assign indices sorted on frequency.
  session.setState(CalSession::INITIALIZING);
  createWorkerIndex(session);

  // Determine the frequency range of the observation.
  const ProcessId firstReducer = session.getWorkerByIndex(CalSession::KERNEL,
    0);
  const ProcessId lastReducer = session.getWorkerByIndex(CalSession::KERNEL,
    session.getWorkerCount(CalSession::KERNEL) - 1);
  pair<double, double> freqRange(session.getFreqRange(firstReducer).start,
    session.getFreqRange(lastReducer).end);
  LOG_INFO_STR("Total frequency range (MHz): [" << freqRange.first / 1e6 << ","
    << freqRange.second / 1e6 << "]");

  // Determine global time axis and verify consistency across all parts.
  Axis::ShPtr timeAxis = getGlobalTimeAxis(session);
  session.setTimeAxis(timeAxis);

  // Apply time range selection.
  pair<size_t, size_t> timeRange = parseTimeRange(timeAxis,
    strategy.timeRange());
  if(timeRange.first > timeRange.second)
  {
    LOG_ERROR_STR("Observation outside the specified time range: "
      << strategy.timeRange());
    session.setState(CalSession::FAILED);
    return 1;
  }
  LOG_INFO_STR("Selected time range (sample): [" << timeRange.first << ","
    << timeRange.second << "] (out of: " << timeAxis->size() << ")");

  size_t chunkStart = timeRange.first, chunkSize = strategy.chunkSize();
  if(chunkSize == 0)
  {
    // If chunk size equals 0, take the whole observation as a single chunk.
    chunkSize = timeRange.second - timeRange.first + 1;
  }
  LOG_INFO_STR("Chunk size (sample): " << chunkSize);

  // Switch session state.
  session.setState(CalSession::PROCESSING);

  // Post an InitializeCommand and wait for all workers to respond.
  CommandId id = session.postCommand(InitializeCommand(strategy));

  CalSession::CommandStatus status = {0, 0};
  size_t nWorker = session.getWorkerCount();
  while(status.finished != nWorker)
  {
    if(session.waitForResult())
    {
      status = session.getCommandStatus(id);
    }
  }

  if(status.failed != 0)
  {
    LOG_ERROR_STR("Worker processes failed to initialize.");
    session.setState(CalSession::FAILED);
    return 1;
  }

  // Session control.
  StrategyIterator it;
  bool done = false;
  while(!done)
  {
    if(!it.atEnd())
    {
      // Post command.
      if((*it)->type() != "Solve")
      {
        session.postCommand(**it, CalSession::KERNEL);
      } else {
        session.postCommand(**it);
      }

      // Advance to the next command.
      ++it;
    }
    else if(chunkStart <= timeRange.second)
    {
      // Move to the next chunk.
      const double start = timeAxis->lower(chunkStart);
      const double end = timeAxis->upper(std::min(chunkStart + chunkSize - 1,
        timeRange.second));

      // Update position.
      chunkStart += chunkSize;

      // Post next chunk command and wait for one or more reducers to respond
      // succes (in other words, continue processing as soon as any reducer
      // responds succes).
      CommandId id = session.postCommand(NextChunkCommand(freqRange.first,
        freqRange.second, start, end), CalSession::KERNEL);

      CalSession::CommandStatus status = {0, 0};
      const size_t nReducer = session.getWorkerCount(CalSession::KERNEL);
      while(status.finished <= status.failed && status.failed < nReducer)
      {
        if(session.waitForResult())
        {
          status = session.getCommandStatus(id);
        }
      }

      if(status.failed == nReducer)
      {
        LOG_ERROR_STR("Reducers processes failed to move to the next chunk.");
        session.setState(CalSession::FAILED);
        return 1;
      }

      // Re-initialize strategy iterator.
      it = StrategyIterator(strategy);
    }
    else
    {
      // Post finalize command and wait for all workers to respond.
      CommandId id = session.postCommand(FinalizeCommand());

      CalSession::CommandStatus status = {0, 0};
      const size_t nWorker = session.getWorkerCount();
      while(status.finished != nWorker)
      {
        if(session.waitForResult())
        {
          status = session.getCommandStatus(id);
        }
      }

      if(status.failed != 0)
      {
        LOG_ERROR_STR("Worker processes failed to finalize.");
        session.setState(CalSession::FAILED);
        return 1;
      }

      // Update session state and flag the current run as done.
      session.setState(CalSession::DONE);
      done = true;
    }
  }

  return 0;
}

bool equal(const Axis::ShPtr &lhs, const Axis::ShPtr &rhs)
{
  if(lhs->size() != rhs->size())
  {
    return false;
  }

  if(lhs->isRegular() && rhs->isRegular())
  {
    return casa::near(lhs->start(), rhs->start()) && casa::near(lhs->end(),
      rhs->end());
  }

  for(size_t i = 0, end = lhs->size(); i < end; ++i)
  {
    if(casa::near(lhs->center(i), rhs->center(i)) && casa::near(lhs->width(i),
      rhs->width(i)))
    {
      continue;
    }

    return false;
  }

  return true;
}

Axis::ShPtr getGlobalTimeAxis(const CalSession &session)
{
  vector<ProcessId> reducers = session.getWorkersByType(CalSession::KERNEL);
  ASSERT(reducers.size() > 0);

  Axis::ShPtr globalAxis;
  for(size_t i = 0; i < reducers.size(); ++i)
  {
    Axis::ShPtr localAxis = session.getTimeAxis(reducers[i]);
    if(!localAxis)
    {
      THROW(BBSControlException, "Time axis not known for reducer process: "
        << reducers[i]);
    }

    if(globalAxis && !equal(globalAxis, localAxis))
    {
      THROW(CalSessionException, "Time axis inconsistent for reducer process: "
        << reducers[i]);
    }
    else
    {
      globalAxis = localAxis;
    }
  }

  return globalAxis;
}

// Compare reducer processes based on start frequency.
struct LessReducer
{
  bool operator()(const pair<ProcessId, double> &lhs,
    const pair<ProcessId, double> &rhs)
  {
    return lhs.second < rhs.second;
  }
};

void createWorkerIndex(CalSession &session)
{
  vector<ProcessId> reducers = session.getWorkersByType(CalSession::KERNEL);

  // Sort reducers processes on start frequency.
  vector<pair<ProcessId, double> > index(reducers.size());
  for(size_t i = 0; i < reducers.size(); ++i)
  {
    index[i] = make_pair(reducers[i], session.getFreqRange(reducers[i]).start);
  }

  stable_sort(index.begin(), index.end(), LessReducer());

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
