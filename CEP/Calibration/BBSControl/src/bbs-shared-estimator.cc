//# bbs-shared-estimator: Application that computes parameter estimates by
//# merging normal equations supplied by multiple bbs-reducer processes.
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
#include <Common/SystemUtil.h>
#include <Common/Exception.h>
#include <BBSControl/Package__Version.h>
#include <BBSControl/CalSession.h>
#include <BBSControl/CommandHandlerEstimator.h>
#include <BBSControl/OptionParser.h>
#include <BBSControl/Util.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler handler(Exception::terminate);

int run(const ParameterSet &options, const OptionParser::ArgumentList &args);

// Application entry point.
int main(int argc, char *argv[])
{
  const string progName = LOFAR::basename(argv[0]);
  INIT_LOGGER(progName);
  LOG_INFO_STR(Version::getInfo<BBSControlVersion>(progName, "other"));

  OptionParser parser;
  parser.addOption("Help", "-h", "--help", "Print usage information and exit.");
  parser.addOptionWithDefault("Range", "-r", "--port-range", "6500:6599",
    "Range of ports to search for a free port on which to start listening");
  parser.addOptionWithDefault("Backlog", "-b", "--backlog", "10", "Maximal"
    " number of pending connections.");
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
    cout << "Usage: " << progName << " [OPTION]..." << endl
      << endl
      << "This distributed LM solver is used in distributed calibration runs."
      " It combines" << endl
      << "the normal equations from multiple bbs-reducer processes and"
      " computes" << endl
      << "new parameters estimates. On start-up the solver will try to join the"
      << endl
      << "distributed calibration run with session key KEY." << endl << endl
      << "Mandatory arguments to long options are mandatory for short options"
          " too." << endl
      << parser.documentation() << endl;
    return 0;
  }

  if(args.size() != 1)
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

int run(const ParameterSet &options, const OptionParser::ArgumentList&)
{
  string key = options.getString("Key");
  CalSession session(key, options.getString("Name"), options.getString("User"),
    options.getString("Password"), options.getString("Host"),
    options.getString("Port"));

  pair<unsigned int, unsigned int> range =
    parseRange(options.getString("Range"));
  SharedEstimator::Ptr solver(new SharedEstimator(range.first,
    options.getInt32("Backlog", 10), range.second - range.first + 1));

  // Poll until control is ready to accept workers.
  LOG_INFO_STR("Waiting for control...");
  while(session.getState() == CalSession::WAITING_FOR_CONTROL)
  {
    sleep(5);
  }

  // Try to register.
  LOG_INFO_STR("Trying to register as worker...");
  if(!session.registerAsSolver(solver->port()))
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

  // Process commands until finished.
  bool wait = false;
  CommandHandlerEstimator handler(makeProcessGroup(session), solver);

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

      // Report the result to the controller.
      session.postResult(command.first, result);

      // If an error occurred, log a descriptive message and exit.
      if(result.is(CommandResult::ERROR))
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
