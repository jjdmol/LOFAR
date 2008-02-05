//#  tAH_TestStorage.cc:
//#
//#  Copyright (C) 2002-2005
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

#include "AH_TestStorage.h"
#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <CEPFrame/Step.h>
#include <Transport/TH_Mem.h>
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CS1_Interface/DH_Visibilities.h>

using namespace LOFAR;
using namespace LOFAR::CS1;

int main (int argc, const char** argv){
  INIT_LOGGER("TestStorage");
  try {
    AH_TestStorage test;
    ACC::APS::ParameterSet ps("CS1.parset");
    test.setParameters(ps);
    test.setarg(argc,argv);
    test.baseDefine();
    test.basePrerun();
    int nrRuns = ps.getInt32("General.NRuns");
    cout << "run " << nrRuns << " times" << endl;
    test.baseRun(nrRuns);
    test.basePostrun();
    test.baseQuit();

  } catch (LOFAR::Exception& e) {
    cerr << "Caught exception: "<< e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception "<< endl;
    exit(1);
  }
  return 0;
}
