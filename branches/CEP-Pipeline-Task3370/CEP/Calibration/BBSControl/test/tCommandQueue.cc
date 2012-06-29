//# tCommandQueue.cc: Test program for the CommandQueue class
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
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/CommandQueue.h>
#include <BBSControl/CommandResult.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/RefitStep.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <stdlib.h>
#include <libgen.h>

using namespace LOFAR;
using namespace LOFAR::BBS;

int main(int /*argc*/, char* argv[])
{
  const char* progName = basename(argv[0]);
  const char* refFile  = "/tmp/tCommandQueue.ref";
  const char* outFile  = "/tmp/tCommandQueue.out";
  const char* parsetFile = "tBBSControl.parset";

//   NextCommandType nextCommand;

  INIT_LOGGER(progName);
  LOG_INFO_STR(progName << " is starting up ...");

  try {

    // Initialize strategy and command queue.
    ParameterSet parset(parsetFile);
    Strategy strategy(parset);
//     CommandQueue queue(getenv("USER"));
    CommandQueue queue(parset.getString("BBDB.Name"),
		       parset.getString("BBDB.Username"),
		       parset.getString("BBDB.Host"),
		       parset.getString("BBDB.Port"));
    CommandQueue::Trigger insert_trig(queue, CommandQueue::Trigger::Command);
    vector< shared_ptr<const Step> > steps = strategy.getAllSteps();
    ofstream ofs;

    // Create data; store into command queue and write to reference file.
    ofs.open(refFile);
    ASSERT(ofs);
    // ASSERT(queue.getStrategy(0.00000001));
    queue.setStrategy(strategy);
    queue.addCommand(InitializeCommand());
    for (uint i = 0; i < steps.size(); ++i) {
      queue.addCommand(*steps[i]);
      ofs << *steps[i] << endl;
    }
    ofs.close();
    ASSERT(ofs);

    // Read steps from command queue and write them to output file.
    ofs.open(outFile);
    ASSERT(ofs);
//     cout << "++++" << endl 
//          << *(queue.getNextCommand()) << endl 
//          << "++++" << endl;
    while(true) {
      NextCommandType command = queue.getNextCommand();
      if (!command.first) break;
      queue.addResult(command.second, CommandResult(), SenderId());
      ofs << *command.first << endl;
    }
    ofs.close();
    ASSERT(ofs);

    string cmd;
    string arg = string(refFile) + " " + string(outFile);

    // Compare reference and output file.
    cmd = "diff -q " + arg;
    LOG_TRACE_FLOW_STR("About to make call `system(" << cmd << ")'");
    int result = system(cmd.c_str());
    LOG_TRACE_FLOW_STR("Return value of system() call: " << result);

    // Clean up the generated files
    cmd = "rm -f " + arg;
    LOG_TRACE_FLOW_STR("About to make call `system(" << cmd << ")'");
//     system(cmd.c_str());

    if (result != 0) {
      LOG_ERROR_STR("Files " << refFile << " and " << outFile << " differ");
      return 1;
    }

  }
  catch (Exception& e) {
    LOG_FATAL_STR(progName << " terminated due to fatal exception!\n" << e);
    return 1;
  }

  LOG_INFO_STR(progName << " terminated successfully.");
  return 0;
}
