//# tSubtract.cc: Test program for Prediffer::subtractData
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/Prediffer.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#ifdef AIPS_NO_TEMPLATE_SRC
#include <casa/Arrays/ArrayLogical.cc>   //include .cc for template
#endif
#include <stdexcept>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;
using namespace casa;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doSubtract (Prediffer& pre1, const StepProp& stepProp)
{
  cout << ">>>" << endl;
  ASSERT (pre1.setWorkDomain (0, 1000, 0., 1e12));
  ASSERT (pre1.setStepProp (stepProp));
  cout << "<<<" << endl;
    
  cout << ">>>" << endl;
  pre1.showSettings();
  cout << "<<<" << endl;
  pre1.subtractData();
}


int main (int argc, const char* argv[])
{
  INIT_LOGGER("tSubtract");
  try {
    if (argc < 5) {
      cout << "Run as: tSubtract user msname meqparmtable skyparmtable"
	   << endl;
      return 1;
    }
    // Read the info for the ParmTables
    ParmDB::ParmDBMeta meqPdm("aips", argv[3]);
    ParmDB::ParmDBMeta skyPdm("aips", argv[4]);

    // Do a subtract.
    {
      cout << "Starting subtract test" << endl;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 0, false);
      vector<int> antVec(100);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = i;
      }
      StrategyProp stratProp;
      stratProp.setAntennas (antVec);
      stratProp.setInColumn ("DATA");
      ASSERT (pre1.setStrategyProp (stratProp));
      StepProp stepProp;
      stepProp.setModel (StringUtil::split("REALIMAG.TOTALGAIN.DIPOLE",'.'));
      stepProp.setOutColumn ("CORRECTED_DATA");
      doSubtract (pre1, stepProp);
      cout << "End of subtract test" << endl;

      cout << "Check if CORRECTED_DATA is zero" << endl;
      Table tab(argv[2]);
      ROArrayColumn<Complex> ccol(tab, "CORRECTED_DATA");
      for (uint i=0; i<tab.nrow(); i++) {
	if (! allNear (ccol(i), Complex(), 1e-8)) {
	  cout << ccol(i) << endl;
	  THROW (LOFAR::Exception, "tSubtract: mismatch in row " << i);
	}
      }
    }

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
