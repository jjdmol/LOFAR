//# tDFT.cc: Test program for performance of DFT imaging
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <Common/Timer.h>
#include <casa/BasicSL/Constants.h>
#include <vector>
#include <cmath>
#include <stdexcept>

using namespace LOFAR;
using namespace std;
using namespace casa;

// Do for each visibility the DFT to each l.m point.
// vis * exp(2*pi*(ul+vm+wn))
// The same DFT trick as in BBS can be used:
// - split baseline in stations
// - extend over frequency using scale factor


typedef float    RType;   // data type used for calculations
typedef fcomplex CType;   // data type of the data.

const int ncorr = 4;
const int nstat = 14;
const int ntime = 1440;
const int nfreq = 64;
const int nmap = 20;


void doDFT()
{
  RType totc=0;
  RType tots=0;
  int nbl = nstat*(nstat-1)/2;
  RType lval = 0.01;
  for (int il=0; il<nmap; ++il, lval+=0.001) {
    RType mval = 0.02;
    for (int im=0; im<nmap; ++im, mval+=0.001) {
      RType nval = sqrt(1 - lval*lval - mval*mval);
      RType uval = 1;
      RType vval = 2;
      RType wval = 0.1;
      for (int it=0; it<ntime; ++it) {
	for (int ib=0; ib<nbl; ++ib) {
	  RType val = 6.28 * (lval*uval + mval*vval + nval*wval);
	  RType sinv = sin(val);
	  RType cosv = sinv;       // take care that compiler uses sincos
	  tots += sinv;
	  totc += cosv;
	  uval += 0.1;
	  vval += 0.2;
	  wval += 0.01;
	}
      }
    }
  }
  cout << tots << ' ' << totc << endl;
}

void doDFT2()
{
  RType totc=0;
  RType tots=0;
  int nbl = nstat*(nstat-1)/2;
  RType uval = 1;
  RType vval = 2;
  RType wval = 0.1;
  for (int it=0; it<ntime; ++it) {
    for (int ib=0; ib<nbl; ++ib) {
      RType lval = 0.01;
      RType ulval = uval*lval;
      RType dulval = uval*0.001;
      RType dvmval = vval*0.001;
      for (int il=0; il<nmap; ++il, lval+=0.001, ulval+=dulval) {
	RType mval = 0.02;
	RType ulvmval = ulval + vval*mval;
	for (int im=0; im<nmap; ++im, mval+=0.001, ulvmval+=dvmval) {
	  RType nval = sqrt(1 - lval*lval - mval*mval);
	  RType val = 6.28 * (ulvmval + nval*wval);
	  RType sinv = sin(val);
	  RType cosv = sinv;       // take care that compiler uses sincos
	  tots += sinv;
	  totc += cosv;
	}
      }
      uval += 0.1;
      vval += 0.2;
      wval += 0.01;
    }
  }
  cout << tots << ' ' << totc << endl;
}

int main (int argc, char**)
{
  INIT_LOGGER("tDFT");
  try {
    {
      if (argc > 1) {
	doDFT2();
      } else {
	doDFT();
      }
    }

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
