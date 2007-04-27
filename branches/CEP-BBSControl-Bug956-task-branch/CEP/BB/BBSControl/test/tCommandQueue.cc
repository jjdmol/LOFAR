//#  tCommandQueue.cc: Test program for the CommandQueue class
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/CommandQueue.h>
#include <BBSControl/CommandQueueTrigger.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSShiftStep.h>
#include <BBSControl/BBSRefitStep.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <stdlib.h>
#include <libgen.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

int main(int argc, char* argv[])
{
  const char* progName = basename(argv[0]);
  const char* refFile  = "/tmp/tCommandQueue.ref";
  const char* outFile  = "/tmp/tCommandQueue.out";
  const char* parsetFile = "tBBSControl.parset";

  const Command* command;

  INIT_LOGGER(progName);
  LOG_INFO_STR(progName << " is starting up ...");

  try {

    // Initialize strategy and command queue.
    ParameterSet parset(parsetFile);
    BBSStrategy strategy(parset);
//     CommandQueue queue(getenv("USER"));
    CommandQueue queue("bbs");
    CommandQueueTrigger insert_trig(queue, "insert_command");
    vector<const BBSStep*> steps = strategy.getAllSteps();
    ofstream ofs;

    // Create data; store into command queue and write to reference file.
    ofs.open(refFile);
    ASSERT(ofs);
    // ASSERT(queue.getStrategy(0.00000001));
//     queue.setStrategy(strategy);
    for (uint i = 0; i < steps.size(); ++i) {
      queue.addCommand(*steps[i]);
      ofs << *steps[i] << endl;
    }
    ofs.close();
    ASSERT(ofs);

    // Read steps from command queue and write them to output file.
    ofs.open(outFile);
    ASSERT(ofs);
    queue.getNextCommand();
//     while ((command = queue.getNextCommand()) != 0) {
//       ofs << *command << endl;
//     }
    ofs.close();
    ASSERT(ofs);

    string cmd;
    string arg = string(refFile) + " " + string(outFile);

    // Compare reference and output file.
    cmd = "diff " + arg;
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
