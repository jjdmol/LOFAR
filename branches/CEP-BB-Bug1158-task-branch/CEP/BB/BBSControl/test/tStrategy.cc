//#  tStrategy.cc: test program for the Strategy class
//#
//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/MultiStep.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

// Compare files. Return true if equal, otherwise false.
bool compareFiles(const char* f1, const char* f2)
{
  string cmd = "diff " + string(f1) + " " + string(f2);
  LOG_TRACE_FLOW_STR("About to make call `system(" << cmd << ")'");
  int result = system(cmd.c_str());
  LOG_TRACE_FLOW_STR("Return value of system() call: " << result);
  return (result == 0);
}


int main()
{
  const string progName("tStrategy");
  INIT_LOGGER(progName.c_str());

  try {

    const char* psFile  = "tBBSControl.parset";
    const char* refFile = "tBBSControl.parset.ref";
    const char* outFile = "tBBSControl.parset.out";

    ParameterSet ps(psFile);
    Strategy writtenStrategy(ps);
    writtenStrategy.shouldWriteSteps(true);

    {
      Strategy dummy(writtenStrategy);
    }

    ps.clear();
    cout << writtenStrategy << endl;
    ps << writtenStrategy;
    ps.writeFile(refFile);

    ps.clear();
    ps.adoptFile(refFile);
    Strategy readStrategy;
    ps >> readStrategy;

    ps.clear();
    ps << readStrategy;
    ps.writeFile(outFile);
    
    if (!compareFiles(refFile, outFile)) {
      LOG_ERROR_STR("Files " << refFile << " and " << outFile << " differ");
      return 1;
    }

  } catch (LOFAR::Exception& e) {
    LOG_FATAL_STR(e);
    return 1;
  }

  return 0;
}
