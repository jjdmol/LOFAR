//# t3C343.cc: Program for 3C343 calibration
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

#include <BBS3/Prediffer.h>
#include <BBS3/Solver.h>
#include <BBS3/MNS/MeqStoredParmPolc.h>
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <malloc.h>

using namespace LOFAR;
using namespace std;

// Note:
// 3C343/10008336.MS contains 64 frequency channels of 156250 Hz with
// center frequencies of 1.18-1.17015625 GHz.
// There are 1437 time stamps of 30 sec in it.

void writeParms (const vector<ParmData>& pData, const MeqDomain& domain)
{
  MeqParmGroup pgroup;
  for (uint i=0; i<pData.size(); ++i) {
    cout << "Writing parm " << pData[i].getName() << " into "
	 << pData[i].getTableName() << ' ' << pData[i].getDBName()
	 << " (" << pData[i].getDBType()
	 << ") values=" << pData[i].getValues() << endl;
    ParmTable ptab(pData[i].getDBType(), pData[i].getTableName(),
		   pData[i].getDBName(), "");
    MeqStoredParmPolc parm(pData[i].getName(), &pgroup, &ptab);
    parm.readPolcs (domain);
    parm.update (pData[i].getValues());
    parm.save();
  }
}

void doSolve (Prediffer& pre1, const vector<string>& solv,
	      int niter)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>(), true);
  // Set a domain. Only use center frequency and all times.
  vector<uint32> shape = pre1.setDomain (1.18e9-59.5*156250, 56*156250,
					 0., 1e12);
  uint nrval = shape[0] * shape[1] * shape[2];
  uint bufnreq = shape[2];
  uint totnreq = shape[3];
  uint nrloop = (totnreq + bufnreq - 1) / bufnreq;
  cout << "bufShape " << shape << endl;
  double* buffer = new double[nrval];
  char* flags = new char[shape[0]*shape[2]];
    
  // Get the ParmData from the Prediffer and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (1);
  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
  vector<ParmData> pData = pre1.getSolvableParmData();
  pre1.showSettings();
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(15) << solver.getSolvableValues() << endl;

  for (int it=0; it<niter; ++it) {
    // Get the equations from the prediffer and give them to the solver.
    for (uint i=0; i<nrloop; i++) {
      int nres;
      bool more = pre1.getEquations (buffer, flags, shape, nres);
      int nreq = bufnreq;
      if (i == nrloop-1) {
	nreq = totnreq  - (nrloop-1)*bufnreq;
	ASSERT (!more);
      } else {
	ASSERT (more);
      }
      ASSERT (nres == nreq);
      solver.setEquations (buffer, flags, nreq, shape[1]-1, shape[0], 0);
    }
    // Do the solve.
    Quality quality;
    solver.solve (false, quality);
    cout << "iter" << it << ":  " << setprecision(15)
	 << solver.getSolvableValues() << endl;
    cout.precision (prec);
    pre1.updateSolvableParms (solver.getSolvableParmData());
  }
  delete [] buffer;
  delete [] flags;
}


int main (int argc, const char* argv[])
{
  cout << ">>>" << endl;
  INIT_LOGGER("t3C343");
  try {
    if (argc < 5) {
      cerr << "Run as: t3C343 user msname meqparmtable skyparmtable [nriter=1] [calcuvw=1]"
	   << endl;
      return 1;
    }
    int nriter=1;
    if (argc > 5) {
      istringstream iss(argv[5]);
      iss >> nriter;
    }
    int calcuvw=1;
    if (argc > 6) {
      istringstream iss(argv[6]);
      iss >> calcuvw;
    }
    // Do a solve for StokesI.
    {
      vector<int> antVec(14);
      for (int i=0; i<14; i++) {
	antVec[i] = i;
      }
      Prediffer pre1(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, "LOFAR.AP", calcuvw, true);
      // Do a further selection; only XX and no autocorrelations.
      vector<int> corr(1,0);
      vector<int> antVec2;
      pre1.select (antVec2, antVec2, false, corr);
      vector<string> solv(1);
      solv[0] = "StokesI.*";
      //      solv[1] = "RA.*";
      //      solv[2] = "DEC.*";
      doSolve (pre1, solv, nriter);
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "<<<" << endl;
  return 0;
}
