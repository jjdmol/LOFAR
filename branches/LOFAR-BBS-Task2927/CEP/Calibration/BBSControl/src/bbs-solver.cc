//# bbs-solver: Application that computes parameter estimates by merging normal
//# equations supplied by multiple bbs-data-processor processes.
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
//#include <BBSControl/KernelProcessControl.h>
#include <BBSControl/KernelConnection.h>
#include <BBSControl/SolveTask.h>
#include <BBSControl/OptionParser.h>
#include <BBSControl/CalSession.h>
#include <BBSControl/CommandProcessorSolver.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/ParmManager.h>
#include <BBSControl/Messages.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/Package__Version.h>
#include <BBSControl/Util.h>
#include <PLC/ACCmain.h>
#include <Common/Exception.h>
#include <Common/StreamUtil.h>
#include <cstdlib>

using namespace LOFAR;
using namespace LOFAR::BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

int main(int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.appendOption("Help", "-h", "--help", "Print usage information"
    " and exit.");
  parser.appendOptionWithDefault("Range", "-r", "--port-range", "6500:6599",
    "Range of ports to search for a free port on which to start listening");
  parser.appendOptionWithDefault("Backlog", "-b", "--backlog", "10", "Maximal"
    " number of pending connections.");
  parser.appendOptionWithDefault("Key", "-k", "--key", "default", "Session"
    " key.");
  parser.appendOptionWithDefault("Name", "-d", "--db-name",
    (getenv("USER") ? : ""), "Name of the database used to store shared"
    " state.");
  parser.appendOptionWithDefault("Host", "-H", "--db-host", "localhost",
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
    cout << "Usage: " << progName << " [OPTION]..." << endl
      << "Calibrate MS blablabla asdasd asdasd asd" << endl << endl
      << "Mandatory arguments to long options are mandatory for short options"
          " too." << endl
      << parser.documentation() << endl << endl
      << "blablabla blabal" << endl;
    return 0;
  }

  if(args.size() != 1)
  {
    cerr << "error: wrong number of arguments." << endl
      << "Try " << progName << " --help for more information" << endl;
    return 1;
  }

  string key = options.getString("Key");
  CalSession session(key,
      options.getString("Name"),
      options.getString("User"),
      options.getString("Password"),
      options.getString("Host"),
      options.getString("Port"));

  pair<unsigned int, unsigned int> range =
    parseRange(options.getString("Range"));
  DistributedLMSolver::Ptr solver(new DistributedLMSolver(range.first,
    options.getInt32("Backlog", 10), range.second - range.first + 1));

  // Poll until Control is ready to accept workers.
  LOG_INFO_STR("Waiting for Control...");
  while(session.getState() == CalSession::WAITING_FOR_CONTROL) {
    sleep(3);
  }

  // Try to register as solver.
  if(!session.registerAsSolver(solver->port())) {
    LOG_ERROR_STR("Could not register as solver. There may be stale"
      " state in the database for key: " << key);
    return 1;
  }

  LOG_INFO_STR("Registration OK.");

  LOG_DEBUG_STR("Waiting for workers...");
  // Wait for workers to register.
  while(session.getState() <= CalSession::INITIALIZING) {
    sleep(3);
  }
  LOG_DEBUG_STR("Workers ready.");


  CommandProcessorSolver processor(makeProcessGroup(session), solver);

  bool wait = false;
  while(!processor.hasFinished()) {
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
        return 1;
      }
    }
    else {
      wait = true;
    }
  }

  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}
