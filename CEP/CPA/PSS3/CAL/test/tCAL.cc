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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <CAL/MeqCalibraterImpl.h>
#include <Common/lofar_strstream.h>
#include <aips/Exceptions/Error.h>
#include <aips/Arrays/Vector.h>
#include <aips/Arrays/ArrayUtil.h>
#include <Common/Profiling/PerfProfile.h>

void predict (MeqCalibrater& mc)
{
  mc.resetIterator();
  while (mc.nextInterval()) {
    mc.predict ("MODEL_DATA");
  }
}

void setsolvable(MeqCalibrater& mc)
{
  Vector<String> parmPatterns(3);
  Vector<String> excludePatterns(0);

  mc.clearSolvableParms();

  parmPatterns[0] = "GSM.*.I";
  parmPatterns[1] = "GSM.*.DEC";
  parmPatterns[2] = "GSM.*.RA";
  mc.setSolvableParms(parmPatterns, excludePatterns, true);
}

void solve(MeqCalibrater& mc, int loopcnt)
{
  double fit;

  mc.resetIterator();
  while (mc.nextInterval()) {
    for (int i = 0; i < loopcnt; i++)
    {
      PERFPROFILE("solve");

      cout << "solve " << endl;
      GlishRecord rec = mc.solve("MODEL_DATA");
      GlishArray(rec.get("fit")).get(fit);
      cout << "fit = " << fit << endl;
    }
  }
}

int main(int argc, char* argv[])
{
  PerfProfile::init(&argc, &argv);

  try {
    if (argc < 2) {
      cerr << "Run as:  tCAL msname mepname gsmname scenario loopcnt "
	   << "stchan endchan selection calcuvw" << endl;
      cerr << " msname:    name of MS without .MS extension" << endl;
      cerr << " mepname:   name of MEP table   (default is msname)" << endl;
      cerr << " gsmname:   name of GSM table   (default is msname)" << endl;
      cerr << " modeltype: WSRT or LOFAR       (default is LOFAR)" << endl;
      cerr << " scenario:  scenario to execute (default is predict)" << endl;
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
      gsmname = msname;
    }

    string modelType;
    if (argc > 4) {
      modelType = argv[4];
    }
    if ("0" == modelType  ||  "" == modelType) {
      modelType = "LOFAR";
    }

    string scenario;
    if (argc > 5) {
      scenario = argv[5];
    }
    if ("0" == scenario  ||  "" == scenario) {
      scenario = "predict";
    }

    int loopcnt = 1;
    if (argc > 6) {
      loopcnt = atoi(argv[6]);
      if (0 == loopcnt) loopcnt = 1;
    }

    int stchan = -1;
    if (argc > 7) {
      istringstream iss(argv[7]);
      iss >> stchan;
    }
    int endchan = stchan;
    if (argc > 8) {
      istringstream iss(argv[8]);
      iss >> endchan;
    }

    string selstr;
    if (argc > 9) {
      selstr = argv[9];
    }

    string peelstr;
    if (argc > 10) {
      peelstr = argv[10];
    }
    Vector<Int> peelVec;
    if (!peelstr.empty()) {
      Vector<String> peels = stringToVector (peelstr);
      peelVec.resize (peels.nelements());
      for (unsigned int i=0; i<peels.nelements(); i++) {
	istringstream iss(peels(i));
	iss >> peelVec(i);
      }
    }

    bool calcuvw=true;
    if (argc > 11) {
      istringstream iss(argv[11]);
      iss >> calcuvw;
    }

    cout << "tCAL parameters:" << endl;
    cout << " msname:    " << msname << endl;
    cout << " mepname:   " << mepname << endl;
    cout << " gsmname:   " << gsmname << endl;
    cout << " modeltype: " << modelType << endl;
    cout << " scenario:  " << scenario << endl;
    cout << " loopcnt:   " << loopcnt << endl;
    cout << " stchan:    " << stchan << endl;
    cout << " endchan:   " << endchan << endl;
    cout << " selection: " << selstr << endl;
    cout << " peel:      " << peelVec << endl;
    cout << " calcuvw  : " << calcuvw << endl;

    Vector<Int> ant;
    MeqCalibrater meqcal (msname+".MS", mepname, gsmname, 0, ant, ant,
			  modelType, calcuvw);

    if (stchan >= 0  ||  endchan >= 0  ||  !selstr.empty()) {
      if (stchan < 0) {
	stchan = 0;
      }
      if (endchan < 0) {
	endchan = meqcal.getNrChan();
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
	meqcal.peel (peelVec);
	for (int i = 0; i < loopcnt; i++)
	{
	  PERFPROFILE("predict");
	  predict (meqcal);
	}
      } else if ("solve" == scenario) {
	meqcal.peel (peelVec);
	setsolvable (meqcal);
	solve (meqcal, loopcnt);
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
