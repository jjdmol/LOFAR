//# bbs-data-processor: Application that processes visibility data, either
//# stand-alone or in co-operation with other bbs_data-processor processes in a
//# distributed calibration run.
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
#include <BBSControl/CommandProcessorCore.h>
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

#ifdef HAVE_PQXX
#include <BBSControl/CalSession.h>
#endif

using namespace LOFAR;
using namespace LOFAR::BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

#ifdef HAVE_PQXX
bool runDistributed(const OptionParser::ArgumentList &args,
  const ParameterSet &options)
{
  MeasurementAIPS::Ptr ms(new MeasurementAIPS(args[2]));

  SourceDB sourceDB(ParmDBMeta("casa", options.getString("SourceDB", args[2]
    + "/sky")));
  ParmManager::instance().initCategory(SKY, sourceDB.getParmDB());

  ParmDB parmDB(ParmDBMeta("casa", options.getString("ParmDB",
    args[2] + "/instrument")));
  ParmManager::instance().initCategory(INSTRUMENT, parmDB);

  string key = options.getString("Key");
  CalSession session(key,
      options.getString("Name"),
      options.getString("User"),
      options.getString("Password"),
      options.getString("Host"),
      options.getString("Port"));

  // Poll until Control is ready to accept workers.
  LOG_DEBUG_STR("Waiting for control...");
  while(session.getState() == CalSession::WAITING_FOR_CONTROL) {
    sleep(3);
  }
  LOG_DEBUG_STR("Control ready.");

  // Try to register as kernel.
  if(!session.registerAsKernel(args[1], args[2], ms->grid()[FREQ],
    ms->grid()[TIME])) {
    LOG_ERROR_STR("Could not register as kernel. There may be stale state"
      " in the database for key: " << key);
    return false;
  }
  LOG_INFO_STR("Registration OK.");

  // Get the global ParameterSet and write it into the HISTORY table.
  ms->writeHistory(session.getParset());

  LOG_DEBUG_STR("Waiting for workers...");
  // Wait for workers to register.
  while(session.getState() <= CalSession::INITIALIZING) {
    sleep(3);
  }
  LOG_DEBUG_STR("Workers ready.");

  CommandProcessorCore processor(makeProcessGroup(session), ms, parmDB,
    sourceDB);

  bool wait = false;
  while(!processor.hasFinished()) {
    LOG_DEBUG_STR("start waiting...");

    if(wait) {
      if(!session.waitForCommand()) {
        continue;
      }

      wait = false;
    }

    pair<CommandId, shared_ptr<const Command> > command = session.getCommand();
    if(command.second) {
      LOG_DEBUG_STR("Executing a " << command.second->type()
        << " command:" << endl << *(command.second));

      // Try to execute the command.
      CommandResult result = command.second->accept(processor);

      // Report the result to the global controller.
      session.postResult(command.first, result);

      // If an error occurred, log a descriptive message and exit.
      if(result.is(CommandResult::ERROR)) {
        LOG_ERROR_STR("Error executing " << command.second->type()
          << " command: " << result.message());
        return false;
      }
    }
    else {
      wait = true;
    }
  }

  return false;
}
#endif

bool runStandAlone(const OptionParser::ArgumentList &args,
  const ParameterSet &options)
{
  MeasurementAIPS::Ptr ms(new MeasurementAIPS(args[1]));

  SourceDB sourceDB(ParmDBMeta("casa", options.getString("SourceDB", args[1]
    + "/sky")));
  ParmManager::instance().initCategory(SKY, sourceDB.getParmDB());

  ParmDB parmDB(ParmDBMeta("casa", options.getString("ParmDB",
    args[1] + "/instrument")));
  ParmManager::instance().initCategory(INSTRUMENT, parmDB);

  ParameterSet parset(args[2]);
  Strategy strategy(parset);
  ASSERT(!strategy.useSolver());

  // Get the global ParameterSet and write it into the HISTORY table.
  ms->writeHistory(parset);

  Axis::ShPtr itsTimeAxis(ms->grid()[TIME]);
  double itsTimeStart = 0;
  double itsTimeEnd = ms->grid()[TIME]->size() - 1;

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

  pair<double, double> fRange = ms->grid()[FREQ]->range();
  pair<double, double> tRange = ms->grid()[TIME]->range();
  Process process = {ProcessId::id(), 0, Interval<double>(fRange.first,
    fRange.second), Interval<double>(tRange.first, tRange.second)};
  ProcessGroup group;
  group.push_back(ProcessGroup::KERNEL, process);

  CommandProcessorCore processor(group, ms, parmDB, sourceDB);

  InitializeCommand initCmd(strategy);
  initCmd.accept(processor);

  StrategyIterator it;
  while(!processor.hasFinished()) {
    if(!it.atEnd())
    {
      (*it)->accept(processor);
      ++it;
    }
    else
    {
      LOG_DEBUG_STR("itsChunkStart: " << itsChunkStart << " itsTimeEnd: " << itsTimeAxis->size() - 1);

      if(itsChunkStart >= (ms->grid()[TIME]->size()))
      {
        FinalizeCommand finCmd;
        finCmd.accept(processor);
      }
      else
      {
        // NEXT_CHUNK
        const double itsTimeEnd = itsTimeAxis->size() - 1;
        ASSERT(itsChunkStart < itsTimeAxis->size());

        const double start = itsTimeAxis->lower(itsChunkStart);
        const double end = itsTimeAxis->upper(std::min(itsChunkStart
          + itsChunkSize - 1, itsTimeEnd));

        itsChunkStart += itsChunkSize;

        Axis::ShPtr itsFreqAxis = ms->grid()[FREQ];
        NextChunkCommand nextCmd(itsFreqAxis->start(), itsFreqAxis->end(), start, end);
        nextCmd.accept(processor);
        it = StrategyIterator(strategy);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.addOption("Help", "-h", "--help", "Print usage information and exit.");
  parser.addOptionWithArgument("SourceDB", "-s", "--sourcedb", "Path to an"
    " alternative source database. By default MS/sky will be used.");
  parser.addOptionWithArgument("ParmDB", "-P", "--parmdb", "Path to an"
    " alternative parameter database. By default MS/instrument will be used.");

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
      << "   Or: " << progName << " [OPTION]... -D FILESYSTEM MS" << endl
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

#ifdef HAVE_PQXX
  if(options.getBool("Distributed", false))
  {
    runDistributed(args, options);
  }
  else
#endif
  {
    runStandAlone(args, options);
  }

  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}
