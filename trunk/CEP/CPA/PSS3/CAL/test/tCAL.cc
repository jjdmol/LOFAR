//# tCAL.cc: Test program for WSRT or LOFAR model type
//# Copyright (C) 2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>

#include <CAL/MeqCalibraterImpl.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableParse.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <Common/Profiling/PerfProfile.h>
#include <sstream>

void predict (MeqCalibrater& mc)
{
  mc.resetIterator();
  while (mc.nextInterval()) {
    mc.predict ("MODEL_DATA");
  }
}

void setsolvable(MeqCalibrater& mc, const String& pattern)
{
  Vector<String> parmPatterns(1);
  Vector<String> excludePatterns(0);

  mc.clearSolvableParms();

  parmPatterns[0] = pattern;
  mc.setSolvableParms(parmPatterns, excludePatterns, true);
}

void solve(MeqCalibrater& mc, int loopcnt, bool realsol)
{
  double fit;

  mc.resetIterator();
  while (mc.nextInterval()) {
    for (int i = 0; i < loopcnt; i++)
    {
      PERFPROFILE("solve");

      cout << "solve " << endl;
      GlishRecord rec = mc.solve(realsol);
      GlishArray(rec.get("fit")).get(fit);
      cout << "fit = " << fit << endl;
    }
  }
}

using namespace LOFAR;

int main(int argc, char* argv[])
{
  // run ../../gnu_opt/demo/michiel.demo 0 ../../gnu_opt/demo/michiel.demo_gsm  LOFAR.RI solve 0 1 0 0 "all([ANTENNA1,ANTENNA2] in 4*[0:20])" 0,1,2 0
  // run ../../gnu_opt/demo/michiel.demo 0 ../../gnu_opt/demo/michiel.demo_gsm  LOFAR.RI solve 0 1 0 0 "all([ANTENNA1,ANTENNA2] in 4*[0:1])" 0,1,2 0
  // run ../../gnu_opt/demo/michiel.demo 0 ../../gnu_opt/demo/michiel.demo_gsm  WSRT solve 0 1 0 0 "all([ANTENNA1,ANTENNA2] in 4*[0:20])" 0,1,2 0

  PerfProfile::init(&argc, &argv, PP_LEVEL_2);

  try {
    if (argc < 2) {
      cerr << "Run as:  tCAL msname mepname gsmname dbtype dbname scenario "
	   << "loopcnt stchan endchan selection calcuvw" << endl;
      cerr << " msname:    name of MS without .MS extension" << endl;
      cerr << " mepname:   name of MEP table   (default is msname)" << endl;
      cerr << " gsmname:   name of GSM table   (default is msname)" << endl;
      cerr << " dbtype:    database type       ([aips], postgres)" << endl;
      cerr << " dbname:    database name       (default is test)" << endl;
      cerr << " modeltype: WSRT or LOFAR       (default is LOFAR)" << endl;
      cerr << " scenario:  scenario to execute (default is predict)" << endl;
      cerr << " solvparms: solvable parms pattern ({RA,DEC,StokesI}.*)"
	   << endl;
      cerr << " loopcnt:   number of scenario loops (default is 1)" << endl;
      cerr << " stchan:    first channel       (default is -1 (first channel)"
	   << endl;
      cerr << " endchan:   last channel        (default is stchan)" << endl;
      cerr << " selection: TaQL selection string (default is empty)" << endl;
      cerr << " peel:      source nrs to peel as 2,4,1 (default is all)"
	   << endl;
      cerr << " calcuvw:   calculate UVW       (default is 1)" << endl;
      return 0;
    }
    string msname (argv[1]);

    string mepname;
    if (argc > 2) {
      mepname = argv[2];
    }
    if ("0" == mepname  ||  "" == mepname) {
      mepname = msname;
    }

    string gsmname;
    if (argc > 3) {
      gsmname = argv[3];
    }
    if ("0" == gsmname  ||  "" == gsmname) {
      gsmname = mepname;
    }

    string dbtype;
    if (argc > 4) {
      dbtype = argv[4];
    }
    if ("0" == dbtype  ||  "" == dbtype) {
      dbtype = "aips";
    }

    string dbname;
    if (argc > 5) {
      dbname = argv[5];
    }
    if ("0" == dbname  ||  "" == dbname) {
      dbname = "test";
    }

    string modelType;
    if (argc > 6) {
      modelType = argv[6];
    }
    if ("0" == modelType  ||  "" == modelType) {
      modelType = "LOFAR";
    }

    string scenario;
    if (argc > 7) {
      scenario = argv[7];
    }
    if ("0" == scenario  ||  "" == scenario) {
      scenario = "predict";
    }

    string solvparms;
    if (argc > 8) {
      solvparms = argv[8];
    }
    if ("0" == solvparms  ||  "" == solvparms) {
      solvparms = "{RA,DEC,StokesI}.*";
    }

    int loopcnt = 1;
    if (argc > 9) {
      loopcnt = atoi(argv[9]);
      if (0 == loopcnt) loopcnt = 1;
    }

    int stchan = -1;
    if (argc > 10) {
      std::istringstream iss(argv[10]);
      iss >> stchan;
    }
    int endchan = stchan;
    if (argc > 11) {
      std::istringstream iss(argv[11]);
      iss >> endchan;
    }

    string selstr;
    if (argc > 12) {
      selstr = argv[12];
    }

    string peelstr;
    if (argc > 13) {
      peelstr = argv[13];
    }
    Vector<Int> peelVec;
    if (!peelstr.empty()) {
      Vector<String> peels = stringToVector (peelstr);
      peelVec.resize (peels.nelements());
      for (unsigned int i=0; i<peels.nelements(); i++) {
	std::istringstream iss(peels(i));
	iss >> peelVec(i);
      }
    }

    bool calcuvw=true;
    if (argc > 14) {
      std::istringstream iss(argv[14]);
      iss >> calcuvw;
    }

    cout << "tCAL parameters:" << endl;
    cout << " msname:    " << msname << endl;
    cout << " mepname:   " << mepname << endl;
    cout << " gsmname:   " << gsmname << endl;
    cout << " dbtype:    " << dbtype << endl;
    cout << " dbname:    " << dbname << endl;
    cout << " modeltype: " << modelType << endl;
    cout << " scenario:  " << scenario << endl;
    cout << " solvparms: " << solvparms << endl;
    cout << " loopcnt:   " << loopcnt << endl;
    cout << " stchan:    " << stchan << endl;
    cout << " endchan:   " << endchan << endl;
    cout << " selection: " << selstr << endl;
    cout << " peel:      " << peelVec << endl;
    cout << " calcuvw  : " << calcuvw << endl;

    // Get the antennas from the selection.
    Vector<Int> ant1, ant2;
    {
      Table tab;
      if (selstr.empty()) {
	tab = Table(msname+".MS");
      } else {
	tab = tableCommand("SELECT FROM " + msname + ".MS WHERE " + selstr);
      }
      Table sortab = tab.sort ("ANTENNA1", Sort::Ascending,
			       Sort::QuickSort | Sort::NoDuplicates);
      ant1 = ROScalarColumn<Int>(sortab, "ANTENNA1").getColumn();
      sortab = tab.sort ("ANTENNA2", Sort::Ascending,
			 Sort::QuickSort | Sort::NoDuplicates);
      ant2 = ROScalarColumn<Int>(sortab, "ANTENNA2").getColumn();
    }
    MeqCalibrater meqcal (msname+".MS", mepname, gsmname, dbtype, dbname, "",
			  0, ant1, ant2, modelType, calcuvw,
			  "MODEL_DATA", "CORRECTED_DATA");

    if (stchan >= 0  ||  endchan >= 0  ||  !selstr.empty()) {
      if (stchan < 0) {
	stchan = 0;
      }
      if (endchan < 0) {
	endchan = meqcal.getNrChan() - 1;
	cout << " endchan (modified): " << endchan << endl;
      }
      meqcal.select (selstr, stchan, endchan);
    }

    meqcal.setTimeInterval (3600);
    meqcal.clearSolvableParms();

    if (loopcnt <= 0)
    {
      cerr << "loopcnt must be > 0" << endl;
    }
    else
    {
      if ("predict" == scenario) {
	for (int i = 0; i < loopcnt; i++)
	{
	  PERFPROFILE("predict");
	  predict (meqcal);
	}
      } else if ("solve" == scenario) {
	meqcal.peel (peelVec, Vector<Int>());
	setsolvable (meqcal, solvparms);
	solve (meqcal, loopcnt, true);
      } else if ("saveresidual" == scenario) {
	meqcal.saveResidualData();
      } else {
	cerr << "Invalid scenario; valid are:  predict,solve" << endl;
      }
    }
  } catch (AipsError& x) {
    cerr << "Unexpected AIPS++ exception: " << x.getMesg() << endl;
    return 1;
  } catch (std::exception& x) {
    cerr << "Unexpected other exception: " << x.what() << endl;
    return 1;
  }
  cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
       << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
       << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;
  cout << "OK" << endl;

  PerfProfile::finalize();

  return 0;
}
