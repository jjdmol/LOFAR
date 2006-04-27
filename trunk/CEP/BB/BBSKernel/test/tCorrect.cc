//# tCorrect.cc: Test program for Prediffer::correctData
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
#include <BBS/Prediffer.h>
#include <Common/LofarLogger.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/ArrayLogical.cc>   //include .cc for template
#include <stdexcept>

using namespace LOFAR;
using namespace std;
using namespace casa;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doCorrect (Prediffer& pre1)
{
  cout << ">>>" << endl;
  pre1.setDomain (0, 1000, 0., 1e12);
  cout << "<<<" << endl;
    
  cout << ">>>" << endl;
  pre1.showSettings();
  cout << "<<<" << endl;
  pre1.correctData ("DATA", "CORRECTED_DATA", true);
}


int main (int argc, const char* argv[])
{
  INIT_LOGGER("tCorrect");
  try {
    if (argc < 5) {
      cout << "Run as: tCorrect user msname meqparmtable skyparmtable"
	   << endl;
      return 1;
    }
    // Read the info for the ParmTables
    ParmDB::ParmDBMeta meqPdm("aips", argv[3]);
    ParmDB::ParmDBMeta skyPdm("aips", argv[4]);

    // Do a correct.
    {
      cout << "Starting correct test" << endl;
      vector<int> antVec(100);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = i;
      }
      vector<vector<int> > srcgrp;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ", srcgrp, false);
      doCorrect (pre1);
      cout << "End of correct test" << endl;

      cout << "Check if CORRECTED_DATA matches DATA" << endl;
      Table tab(argv[2]);
      ROArrayColumn<Complex> dcol(tab, "DATA");
      ROArrayColumn<Complex> ccol(tab, "CORRECTED_DATA");
      for (uint i=0; i<tab.nrow(); i++) {
	if (! allNear (dcol(i), ccol(i), 1e-8)) {
	  THROW (LOFAR::Exception, "tCorrect: mismatch in row " << i);
	}
      }
    }

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
