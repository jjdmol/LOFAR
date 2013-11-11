//# bbs-reducer: Application that processes visibility data, either stand-alone
//# or in co-operation with other bbs-reducer processes in a distributed
//# calibration run.
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
#include <BBSControl/CommandHandlerReducer.h>
#include <BBSControl/OptionParser.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/Util.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/ParmManager.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>

#ifdef HAVE_PQXX
#include <BBSControl/CalSession.h>
#endif

using namespace LOFAR;
using namespace LOFAR::BBS;
using LOFAR::operator<<;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

int run(const ParameterSet &options, const OptionParser::ArgumentList &args);

#ifdef HAVE_PQXX
int runDistributed(const ParameterSet &options,
  const OptionParser::ArgumentList &args);
#endif

// Application entry point.
int main(int argc, char *argv[])
{
  const string progName = LOFAR::basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.addOption("Help", "-h", "--help", "Print usage information and exit.");
  parser.addOptionWithArgument("SourceDB", "-s", "--sourcedb", "Path to an"
    " alternative source database. By default MS/sky will be used.");
  parser.addOptionWithArgument("ParmDB", "-P", "--parmdb", "Path to an"
    " alternative parameter database. By default MS/instrument will be used.");
  parser.addOptionWithArgument("LogPath", "-l", "--log-path", "Path where"
    " solver statistic logs are stored. By default MS/ is used.");

#ifdef HAVE_PQXX
  parser.addOption("Distributed", "-D", "--distributed", "Run in distributed"
    " mode, as part of the distributed calibration session identified by"
    " session key KEY.");
  parser.addOptionWithDefault("Key", "-k", "--key", "default", "Session key"
    " (distributed runs only).");
  parser.addOptionWithDefault("Name", "-d", "--db-name",
    (getenv("USER") ? : ""), "Name of the database used to store shared state"
    " (distributed runs only).");
  parser.addOptionWithDefault("Host", "-H", "--db-host", "localhost", "Hostname"
    " of the machine that runs the database server (distributed runs only).");
  parser.addOptionWithDefault("Port", "-p", "--db-port", "5432", "Port on which"
    " the database server is listening (distributed runs only).");
  parser.addOptionWithDefault("User", "-U", "--db-user", "postgres", "Username"
    " used for authentication (distributed runs only).");
  parser.addOptionWithDefault("Password", "-w", "--db-password", "", "Password"
    " used for authentication (distributed runs only).");
#endif

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
    cout << "Usage: " << progName << " [OPTION]... MS PARSET" << endl
#ifdef HAVE_PQXX
      << "   Or: " << progName << " [OPTION]... -D FILESYSTEM MS PARSET" << endl
#endif
      << endl
      << "Calibrate a single MS using the reduction strategy described by"
      " PARSET. The" << endl
      << "second form instead tries to join a distributed calibration"
      " run with session" << endl
      << "key KEY. In this case the reduction is controlled"
      " remotely by a bbs-controller" << endl
      << "process." << endl << endl
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
    int status = 0;
#ifdef HAVE_PQXX
    if(options.getBool("Distributed", false))
    {
      status = runDistributed(options, args);
    }
    else
#endif
    {
      status = run(options, args);
    }

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
  casa::Path pathMS(args[1]);
  MeasurementAIPS::Ptr ms(new MeasurementAIPS(pathMS.absoluteName()));

  casa::Path pathSourceDB(pathMS);
  pathSourceDB.append("sky");
  SourceDB sourceDB(ParmDBMeta("casa", options.getString("SourceDB",
    pathSourceDB.absoluteName())));
  ParmManager::instance().initCategory(SKY, sourceDB.getParmDB());

  casa::Path pathParmDB(pathMS);
  pathParmDB.append("instrument");
  ParmDB parmDB(ParmDBMeta("casa", options.getString("ParmDB",
    pathParmDB.absoluteName())));
  ParmManager::instance().initCategory(INSTRUMENT, parmDB);

  ParameterSet parset(args[2]);
  Strategy strategy(parset);
  if(strategy.useSolver())
  {
    LOG_ERROR_STR("It is not possible to compute global parameter estimates in"
      " stand-alone mode. Please change the reduction strategy or run in"
      " distributed mode.");
    return 1;
  }

  // Set ParmDB default resolution.
  if(ms->nFreq() != 0 && ms->nTime() != 0)
  {
    vector<double> resolution(2);
    resolution[0] = ms->grid()[FREQ]->end() - ms->grid()[FREQ]->start();
    resolution[1] = ms->grid()[TIME]->width(0);
    parmDB.setDefaultSteps(resolution);
  }

  // Get the global ParameterSet and write it into the HISTORY table.
  ms->writeHistory(parset);

  // Apply time range selection.
  pair<size_t, size_t> timeRange = parseTimeRange(ms->grid()[TIME],
    strategy.timeRange());
  if(timeRange.first > timeRange.second)
  {
    LOG_ERROR_STR("Observation outside the specified time range: "
      << strategy.timeRange());
    return 1;
  }
  LOG_INFO_STR("Selected time range (sample): [" << timeRange.first << ","
    << timeRange.second << "] (out of: " << ms->nTime() << ")");

  size_t chunkStart = timeRange.first, chunkSize = strategy.chunkSize();
  if(chunkSize == 0)
  {
    // If chunk size equals 0, take the whole observation as a single chunk.
    chunkSize = timeRange.second - timeRange.first + 1;
  }
  LOG_INFO_STR("Chunk size (sample): " << chunkSize);

  // Construct process group that consists of a single process (this process).
  ProcessGroup group;
  pair<double, double> msFreqRange = ms->grid()[FREQ]->range();
  pair<double, double> msTimeRange = ms->grid()[TIME]->range();
  group.appendReducerProcess(ProcessId::id(),
    Interval<double>(msFreqRange.first, msFreqRange.second),
    Interval<double>(msTimeRange.first, msTimeRange.second));

  casa::Path logPath(options.getString("LogPath", args[1]));
  CommandHandlerReducer handler(group, ms, parmDB, sourceDB, logPath);

  // Fake initialization.
  InitializeCommand initCmd(strategy);
  CommandResult initResult = initCmd.accept(handler);
  if(!initResult)
  {
    LOG_ERROR_STR("Error executing " << initCmd.type() << " command: "
      << initResult.message());
    return 1;
  }

  // Fake session control.
  StrategyIterator it;
  while(!handler.hasFinished())
  {
    if(!it.atEnd())
    {
      // Execute the current command.
      LOG_DEBUG_STR("Executing a " << (*it)->type() << " command:" << endl
        << **it);
      CommandResult result = (*it)->accept(handler);

      if(!result)
      {
        LOG_ERROR_STR("Error executing " << (*it)->type() << " command: "
          << result.message());
        return 1;
      }

      // Move to the next command in the strategy.
      ++it;
    }
    else
    {
      // Reached end of strategy, so move to the next chunk (if any).
      if(chunkStart > timeRange.second)
      {
        // Fake finalization.
        FinalizeCommand finCmd;

        LOG_DEBUG_STR("Executing a " << finCmd.type() << " command:" << endl
          << finCmd);
        CommandResult result = finCmd.accept(handler);

        if(!result)
        {
          LOG_ERROR_STR("Error executing " << finCmd.type() << " command: "
            << result.message());
          return 1;
        }
      }
      else
      {
        ASSERT(chunkStart <= timeRange.second);
        const double start = ms->grid()[TIME]->lower(chunkStart);
        const double end = ms->grid()[TIME]->upper(std::min(chunkStart
          + chunkSize - 1, timeRange.second));

        // Update position.
        chunkStart += chunkSize;

        // Fake next chunk.
        NextChunkCommand nextCmd(msFreqRange.first, msFreqRange.second, start,
          end);

        LOG_DEBUG_STR("Executing a " << nextCmd.type() << " command:" << endl
          << nextCmd);
        CommandResult result = nextCmd.accept(handler);

        if(!result)
        {
          LOG_ERROR_STR("Error executing " << nextCmd.type() << " command: "
            << result.message());
          return 1;
        }

        // Re-initialize strategy iterator.
        it = StrategyIterator(strategy);
      }
    }
  }

  return 0;
}

#ifdef HAVE_PQXX
int runDistributed(const ParameterSet &options,
  const OptionParser::ArgumentList &args)
{
  casa::Path pathMS(args[2]);
  MeasurementAIPS::Ptr ms(new MeasurementAIPS(pathMS.absoluteName()));

  casa::Path pathSourceDB(pathMS);
  pathSourceDB.append("sky");
  SourceDB sourceDB(ParmDBMeta("casa", options.getString("SourceDB",
    pathSourceDB.absoluteName())));
  ParmManager::instance().initCategory(SKY, sourceDB.getParmDB());

  casa::Path pathParmDB(pathMS);
  pathParmDB.append("instrument");
  ParmDB parmDB(ParmDBMeta("casa", options.getString("ParmDB",
    pathParmDB.absoluteName())));
  ParmManager::instance().initCategory(INSTRUMENT, parmDB);

  string key = options.getString("Key");
  CalSession session(key, options.getString("Name"), options.getString("User"),
    options.getString("Password"), options.getString("Host"),
    options.getString("Port"));

  // Poll until control is ready to accept workers.
  LOG_INFO_STR("Waiting for control...");
  while(session.getState() == CalSession::WAITING_FOR_CONTROL)
  {
    sleep(5);
  }

  // Try to register.
  LOG_INFO_STR("Trying to register as worker...");
  if(!session.registerAsKernel(args[1], args[2], ms->grid()[FREQ],
    ms->grid()[TIME]))
  {
    LOG_ERROR_STR("Unable to register. There may be stale state in the database"
      " for the session with session key: " << key);
    return 1;
  }
  LOG_INFO_STR("Registration OK.");

  // Poll until all workers have registered.
  LOG_INFO_STR("Waiting for workers to register...");
  while(session.getState() <= CalSession::INITIALIZING)
  {
    sleep(5);
  }
  LOG_INFO_STR("All workers have registered.");

  // Get the global ParameterSet and write it into the HISTORY table.
  ms->writeHistory(session.getParset());

  // Process commands until finished.
  bool wait = false;
  casa::Path logPath(options.getString("LogPath", args[2]));
  CommandHandlerReducer handler(makeProcessGroup(session), ms, parmDB, sourceDB,
    logPath);

  while(!handler.hasFinished())
  {
    // Wait for new commands (with timeout).
    if(wait)
    {
      if(!session.waitForCommand())
      {
        continue;
      }

      wait = false;
    }

    pair<CommandId, shared_ptr<const Command> > command = session.getCommand();
    if(command.second)
    {
      LOG_DEBUG_STR("Executing a " << command.second->type() << " command:"
        << endl << *(command.second));

      // Try to execute the command.
      CommandResult result = command.second->accept(handler);

      // Report the result to the global controller.
      session.postResult(command.first, result);

      // If an error occurred, log a descriptive message and exit.
      if(!result)
      {
        LOG_ERROR_STR("Error executing " << command.second->type()
          << " command: " << result.message());
        return 1;
      }
    }
    else
    {
      // No commands available, so start waiting.
      wait = true;
    }
  }

  return 0;
}
#endif
