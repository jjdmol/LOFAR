//# tPredSolv.cc: Test program for Prediffer and Solver classes
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
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <malloc.h>

#include <Common/hexdump.h>

#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufChar.h>


using namespace LOFAR;
using namespace std;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doSolve (Prediffer& pre1, const vector<string>& solv)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>(), true);
  // Set a domain.
  //    vector<uint32> shape = pre1.setDomain (137750000-250000, 2*500000,
    vector<uint32> shape = pre1.setDomain (137750000-250000, 4*500000,
  //vector<uint32> shape = pre1.setDomain (0., 1e12,
					 0., 1e12);
  uint nrval = shape[0] * shape[1] * shape[2];
  uint bufnreq = shape[2];
  uint totnreq = shape[3];
  uint nrloop = (totnreq + bufnreq - 1) / bufnreq;
  cout << "bufShape " << shape << endl;
  double* buffer = new double[nrval];
    
  // Get the ParmData from the Prediffer and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (1);
  vector<ParmData> pData = pre1.getSolvableParmData();
  cout << "***Parm data: [ ";
  for (uint j=0; j<pData.size(); j++)
    {
      cout << pData[j].getValues() << " ";
    }
  cout << "]" << endl;

  solver.setSolvableParmData(pData, 0);

  pre1.showSettings();

    //  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);

  // Get the equations from the prediffer and give them to the solver.
  for (uint i=0; i<nrloop; i++) {
    bool more = pre1.getEquations (buffer, shape);
    uint nreq = bufnreq;
    if (i == nrloop-1) {
      nreq = totnreq  - (nrloop-1)*bufnreq;
      ASSERT (!more);
    } else {
      ASSERT (more);
    }
    cout << "*** buffer " << i << " ***" << endl;
    hexdump(buffer, nrval);
    solver.setEquations (buffer, nreq, shape[1]-1, shape[0], 0);
  }

  // Do the solve.
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues() << endl;
  Quality quality;
  solver.solve (false, quality);
  cout << "After:  " << setprecision(10) << solver.getSolvableValues() << endl;
  cout.precision (prec);
  delete [] buffer;
}

void doSolve2 (Prediffer& pre1, Prediffer& pre2, const vector<string>& solv)
{
  Solver solver;
  solver.initSolvableParmData (2);
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre2.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>(), true);
  pre2.setSolvableParms (solv, vector<string>(), true);
  // Set a domain.
  ///  vector<uint32> shape = pre2.setDomain (137750000-250000, 2*500000,
  vector<uint32> shape1 = pre1.setDomain (0., 1e12,
					  0., 1e12);
  vector<uint32> shape2 = pre2.setDomain (0., 1e12,
					  0., 1e12);
  // Get the ParmData from the Prediffers and send it to the solver.
  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
  solver.setSolvableParmData (pre2.getSolvableParmData(), 1);
  {
    // Get the equations.
    uint nrval = shape1[0] * shape1[1] * shape1[2];
    uint bufnreq = shape1[2];
    uint totnreq = shape1[3];
    uint nrloop = (totnreq + bufnreq - 1) / bufnreq;
    cout << "bufShape-1 " << shape1 << endl;
    double* buffer = new double[nrval];
    // Get the equations from the prediffer and give them to the solver.
    for (uint i=0; i<nrloop; i++) {
      bool more = pre1.getEquations (buffer, shape1);
      uint nreq = bufnreq;
      if (i == nrloop-1) {
	nreq = totnreq  - (nrloop-1)*bufnreq;
	ASSERT (!more);
      } else {
	ASSERT (more);
      }
      solver.setEquations (buffer, nreq, shape1[1]-1, shape1[0], 0);
    }
    delete [] buffer;
  }
  {
    uint nrval = shape2[0] * shape2[1] * shape2[2];
    uint bufnreq = shape2[2];
    uint totnreq = shape2[3];
    uint nrloop = (totnreq + bufnreq - 1) / bufnreq;
    cout << "bufShape-2 " << shape2 << endl;
    double* buffer = new double[nrval];
    // Get the equations from the prediffer and give them to the solver.
    for (uint i=0; i<nrloop; i++) {
      bool more = pre2.getEquations (buffer, shape2);
      uint nreq = bufnreq;
      if (i == nrloop-1) {
	nreq = totnreq  - (nrloop-1)*bufnreq;
	ASSERT (!more);
      } else {
	ASSERT (more);
      }
      solver.setEquations (buffer, nreq, shape2[1]-1, shape2[0], 1);
    }
    delete [] buffer;
  }

  // Do the solve.
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues() << endl;
  Quality quality;
  solver.solve (false, quality);
  cout << "After:  " << setprecision(10) << solver.getSolvableValues() << endl;
  cout.precision (prec);
}


int main (int argc, const char* argv[])
{
  cout << ">>>" << endl;
  INIT_LOGGER("tPredSolv");
  try {
    if (argc < 5) {
      cerr << "Run as: tPredSolv user msname meqparmtable skyparmtable"
	   << endl;
      return 1;
    }
    // Do a solve for RA using a few stations.
    {
      cout << "Starting first test" << endl;
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      Prediffer pre1(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, "LOFAR.RI", false, true);
      // Do a further selection of a few stations.
      vector<int> antVec2(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      pre1.select (antVec2, antVec2, false);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve (pre1, solv);
      cout << "End of first test" << endl;
    }
    // Do a solve using 2 prediffers.
    {
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      Prediffer pre1(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, "LOFAR.RI", false, true);
      Prediffer pre2(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, "LOFAR.RI", false, true);
      // Do a further selection of a few stations.
      vector<int> antVec2(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      pre1.select (antVec2, antVec2, false);    // no autocorrelations
      pre2.select (antVec2, antVec2, false);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve2 (pre1, pre2, solv);
    }

    // Do a solve using all stations.
    {
      vector<int> antVec(100);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = i;
      }
      Prediffer pre1(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, "LOFAR.RI", false, true);
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve (pre1, solv);
    }

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "<<<" << endl;
  return 0;
}
