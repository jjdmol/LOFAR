// tDFTServer.cc: Test program for the DFTServer
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <DFTServer/DFTServer.h>
#include <DFTServer/DFTAll.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <vector>

using namespace LOFAR;
using namespace std;


bool doReq (DFTAll& dft,
	    double startFreq, double stepFreq, int nfreq,
	    double startTime, double stepTime, int ntime,
	    double startUVW, double stepUVW, double L, double M,
	    int startAnt, int stepAnt, int nant)
{
  int nbaseline = nant*(nant-1)/2;
  vector<int> antinx(100);
  dft.set (startFreq, stepFreq, nfreq,
	   startTime, stepTime, ntime,
	   nant, nbaseline);
  dft.setLM (L, M);
  double* uvw = dft.accessUVW();
  int32* ant = dft.accessAnt();
  int32* ant1 = dft.accessAnt1();
  int32* ant2 = dft.accessAnt2();
  for (int i=0; i<nant; i++) {
    ant[i] = startAnt + i*stepAnt;
    antinx[ant[i]] = i;
    for (int j=i+1; j<nant; j++) {
      *ant1++ = startAnt + i*stepAnt;
      *ant2++ = startAnt + j*stepAnt;
    }
    for (int j=0; j<ntime; j++) {
      double f = (i+1)*(i+1)*(j+1)*(j+1);
      *uvw++ = startUVW + (double)i/nant + f/10./ntime;
      *uvw++ = startUVW + (double)i/nant + f/10./ntime + stepUVW;
      *uvw++ = startUVW + (double)i/nant + f/10./ntime + 2*stepUVW;
    }
  }
  cout << "Do request for nant=" << dft.getRequest().getNAnt()
       << " ntime=" << dft.getRequest().getNTime()
       << " nfreq=" << dft.getRequest().getNFreq()
       << " startfreq=" << dft.getRequest().getStartFreq()
       << " stepfreq=" << dft.getRequest().getStepFreq()
       << " L=" << dft.getRequest().getL()
       << " M=" << dft.getRequest().getM()
       << " N=" << dft.getRequest().getN()
       << " nbasel=" << dft.getRequest().getNBaseline()
       << endl;
  dft.send();
  dft.receive();
  double N = sqrt(1 - L*L - M*M);
  double C = 2.99792458e+08;          // light speed
  double PI2 = 6.283185307179586476925286;
  uvw = dft.accessUVW();
  ant1 = dft.accessAnt1();
  ant2 = dft.accessAnt2();
  const double* rval = dft.getValues();
  double maxdre = 0;
  double maxdim = 0;
  for (int i=0; i<nbaseline; i++) {
    double* uvw1 = uvw + antinx[ant1[i]] * ntime * 3;
    double* uvw2 = uvw + antinx[ant2[i]] * ntime * 3;
    for (int j=0; j<ntime; j++) {
      double a1 = uvw1[0]*L + uvw1[1]*M + uvw1[2]*N;
      double a2 = uvw2[0]*L + uvw2[1]*M + uvw2[2]*N;
#if 0
      cout << "ant1=" << ant1[i] << " ant2=" << ant2[i]
	   << ' ' << antinx[ant1[i]] << ' ' << antinx[ant2[i]]
	   << " time=" << j
	   << " uvw1=" << uvw1[0] << ',' << uvw1[1] << ',' << uvw1[2]
	   << " uvw2=" << uvw2[0] << ',' << uvw2[1] << ',' << uvw2[2]
	   << " a1=" << a1 << " a2=" << a2
	   << endl;
#endif
      double freq = startFreq/C;
      for (int k=0; k<nfreq; k++) {
	complex<double> c2(0, PI2 * (a1-a2) * freq);
	complex<double> c3 = std::exp(c2);
	double dre = std::abs(*rval - c3.real());
	if (dre > maxdre) maxdre = dre;
	///	cout << " freq=" << freq << endl;
	  ///	cout << "  real: " << *rval << ' ' << c3.real() << ' ' << *rval-c3.real()
	  ///	     << endl;
	rval++;
	double dim = std::abs(*rval - c3.imag());
	if (dim > maxdim) maxdim = dim;
	///	cout << "  imag: " << *rval << ' ' << c3.imag() << ' ' << *rval-c3.imag()
	///	     << endl;
	rval++;
	freq += stepFreq/C;
      }
      uvw1 += 3;
      uvw2 += 3;
    }
  }
  cout << "maxdiffre=" << maxdre << ", maxdiffim=" << maxdim << endl;
  return (maxdre<1e-10 && maxdim<1e-10);
}

bool doIt()
{
  DFTAll dft;
  bool ok = true;
  {
    int nant = 4;
    int nfreq = 10;
    int ntime = 5;
    ok = ok && doReq (dft,
		      1e8, 1e6, nfreq,     // start,step,n freq
		      50., 2., ntime,      // start,step,n time
		      1., 0.1,             // start,step uvw
		      0.4, 0.6,            // L,M
		      0, 1, nant);         // start,step,n ant
  }
  {
    int nant = 4;
    int nfreq = 512;
    int ntime = 5;
    ok = ok && doReq (dft,
		      1e8, 1e6, nfreq,     // start,step,n freq
		      50., 2., ntime,      // start,step,n time
		      1., 0.1,             // start,step uvw
		      0.4, 0.6,            // L,M
		      0, 1, nant);         // start,step,n ant
  }
  // Set end.
  dft.quit();
  return ok;
}

int main()
{
  bool ok = true;
  try {
    // Initialize the LOFAR logger
    INIT_LOGGER("tDFTServer.log_prop");
    ok = doIt();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return (ok ? 0:1);
}
