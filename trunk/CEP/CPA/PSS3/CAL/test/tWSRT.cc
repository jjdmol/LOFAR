//# tWSRT.cc: Test program for WSRT model classes
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

#ifdef HAVE_MPI_PROFILER
#define MPICH_SKIP_MPICXX
#include <mpe.h>
#endif

#include <CAL/MeqCalibraterImpl.h>
#include <Common/lofar_strstream.h>
#include <aips/Exceptions/Error.h>

void predict (MeqCalibrater& mc)
{
  mc.resetIterator();
  while (mc.nextInterval()) {
    mc.predict ("MODEL_DATA");
  }
}

int main(int argc, char* argv[])
{
#ifdef HAVE_MPI_PROFILER
  MPI_Init(&argc, &argv);

  int tt_start = MPE_Log_get_event_number();
  int tt_stop  = MPE_Log_get_event_number();

  MPE_Describe_state(tt_start, tt_stop, "Total runtime", "red");

  MPE_Start_log();
  MPE_Log_event(tt_start, 0, "start program");
#endif

  try {
    if (argc < 2) {
      cout << "Run as:  tWSRT msname mepname gsmname scenario "
	   << "stchan endchan selection" << endl;
      cout << " msname:    name of MS without .MS extension" << endl;
      cout << " mepname:   name of MEP table   (default is msname)" << endl;
      cout << " gsmname:   name of GSM table   (default is msname)" << endl;
      cout << " scenario:  scenario to execute (default is predict)" << endl;
      cout << " stchan:    first channel       (default is -1 (first channel)"
	   << endl;
      cout << " endchan:   last channel        (default is stchan)" << endl;
      cout << " selection: TaQL selection string (default is empty)" << endl;
      return 0;
    }
    string msname (argv[1]);

    string mepname;
    if (argc > 2) {
      mepname = argv[2];
    }
    if (mepname.empty()) {
      mepname = msname;
    }

    string gsmname;
    if (argc > 3) {
      gsmname = argv[3];
    }
    if (gsmname.empty()) {
      gsmname = msname;
    }

    string scenario;
    if (argc > 4) {
      scenario = argv[4];
    }
    if (scenario.empty()) {
      scenario = "predict";
    }

    int stchan = -1;
    if (argc > 5) {
      istringstream iss(argv[5]);
      iss >> stchan;
    }
    int endchan = stchan;
    if (argc > 6) {
      istringstream iss(argv[6]);
      iss >> endchan;
    }

    string selstr;
    if (argc > 7) {
      selstr = argv[7];
    }

    Vector<Int> ant;
    MeqCalibrater meqcal (msname+".MS", mepname, gsmname, 0, ant, ant);

    if (stchan >= 0  ||  endchan >= 0  ||  !selstr.empty()) {
      if (stchan < 0) {
	stchan = 0;
      }
      if (endchan < 0) {
	endchan = meqcal.getNrChan();
      }
      meqcal.select (selstr, stchan, endchan);
    }

    cout << "tWSRT parameters:" << endl;
    cout << " msname:    " << msname << endl;
    cout << " mepname:   " << mepname << endl;
    cout << " gsmname:   " << gsmname << endl;
    cout << " scenario:  " << scenario << endl;
    cout << " stchan:    " << stchan << endl;
    cout << " endchan:   " << endchan << endl;
    cout << " selection: " << selstr << endl;

    meqcal.setTimeInterval (3600);
    meqcal.clearSolvableParms();

    if (scenario == "predict") {
      predict (meqcal);
    } else {
      cout << "Invalid scenario; valid are:  predict" << endl;
    }
  } catch (AipsError& x) {
    cout << "Unexpected AIPS++ exception: " << x.getMesg() << endl;
    return 1;
  } catch (std::exception& x) {
    cout << "Unexpected other exception: " << x.what() << endl;
    return 1;
  }
  cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
       << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
       << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;
  cout << "OK" << endl;

#ifdef HAVE_MPI_PROFILER
  MPE_Log_event(tt_stop, 0, "stop program");
  MPI_Finalize();
  //MPE_Finish_log("tWSRT");
#endif

  return 0;
}
