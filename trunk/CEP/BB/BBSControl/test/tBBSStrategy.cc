//#  tBBSStrategy.cc: test program for the BBSStrategy class
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
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSMultiStep.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

int main()
{
  const string progName("tBBSStrategy");
  INIT_LOGGER(progName.c_str());

  try {

    BBSStrategy strategy(ParameterSet("tBBSControl.parset"));
    cout << strategy << endl;

//     vector<const BBSStep*> steps = strategy.getAllSteps();
//     cout << "This strategy contains " << steps.size() << " steps:" << endl;
//     for (uint i = 0; i < steps.size(); ++i) {
//       cout << "  " << steps[i]->getName() << endl;
//     }

  } catch (LOFAR::Exception& e) {
    LOG_FATAL_STR(e);
    return 1;
  }

  return 0;
}
