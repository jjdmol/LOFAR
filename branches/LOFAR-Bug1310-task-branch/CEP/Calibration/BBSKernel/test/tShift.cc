//# tShift.cc: Test program for performance of a phase shift
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

// Define some constants.

typedef dcomplex CalcType;   // data type used for calculations
#define MAKECALCTYPE(re,im) makedcomplex(double(re),double(im))
//typedef fcomplex CalcType;   // data type used for calculations
//#define MAKECALCTYPE(re,im) makefcomplex(float(re),float(im))

typedef dcomplex VisType;    // data type of the data.
//#define MAKEVISTYPE(re,im) makedcomplex(double(re),double(im))
//typedef fcomplex VisType;    // data type of the data.
#define MAKEVISTYPE(re,im) makefcomplex(float(re),float(im))

const int ncorr = 4;
const int nstat = 37;
const int ntime = 18000;
const int nfreq = 64;
const double f0 = 137750000;
const double df = 500000;
const double ras = 2.734;
const double decs = 0.45379;
const double ra0 = 2.733;
const double dec0 = 0.453;
const int nbrick=16;
const int nu=100;
const int nv=100;

// Define fake FFT-ed image patches with their uv-coordinates (in meters).
struct UVBrick {
  VisType visxx[nu*nv];
  VisType visxy[nu*nv];
  VisType visyx[nu*nv];
  VisType visyy[nu*nv];
  double freq;
};

// Fill the UV-brick.
// There is a brick for a few freqs.
// Each brick has some visibilities on a coarse uv-grid.
// So interpolation in freq, u, and v is needed to get a value.
void fillBricks (UVBrick* bricks)
{
  // Add 2 for room for rounding problems.
  double stepf = ((nfreq-1)*df + 2) / (nbrick-1);
  for (int i=0; i<nbrick; i++) {
    bricks[i].freq = f0 - 1 + i*stepf;
    ASSERT (bricks[i].freq > f0-2  &&  bricks[i].freq < f0+(nfreq-1)*df+2);
    VisType* data = bricks[i].visxx;
    for (int id=0; id<nu*nv; ++id) {
      data[id] = MAKEVISTYPE ((id+1)*1e-2, (id+2)*1e-2);
    }
    data = bricks[i].visxy;
    for (int id=0; id<nu*nv; ++id) {
      data[id] = MAKEVISTYPE ((id+1)*1e-2, (id+2)*1e-2);
    }
    data = bricks[i].visyx;
    for (int id=0; id<nu*nv; ++id) {
      data[id] = MAKEVISTYPE ((id+1)*1e-2, (id+2)*1e-2);
    }
    data = bricks[i].visyy;
    for (int id=0; id<nu*nv; ++id) {
      data[id] = MAKEVISTYPE ((id+1)*1e-2, (id+2)*1e-2);
    }
  }
}

void doShift()
{
  cout << "ncorr=" << ncorr
       << ", nfreq=" << nfreq
       << ", nstat=" << nstat
       << " (nbl=" << nstat*(nstat-1)/2 << ")"
       << ", ntime=" << ntime
       << "; nvis=" << double(ncorr)*nfreq*ntime*nstat*(nstat-1)/2
       << endl;
  cout << "nbrick=" << nbrick << ", nu=" << nu << ", nv=" << nv << endl;
  ASSERT (ncorr%4 == 0);
  ASSERT (nbrick>1 && nu>1 && nv>1);

  // Define some fake data.
  vector<VisType> datavec(ncorr*nfreq);
  for (int i=0; i<ncorr*nfreq; ++i) {
    datavec[i] = MAKEVISTYPE(1e-3*i, 1e-4*i);
  }
  VisType* data = &(datavec[0]);

  // Arrays for U,V,W per station per time stamp.
  vector<double> u(nstat*ntime);
  vector<double> v(nstat*ntime);
  vector<double> w(nstat*ntime);
  for (int i=0; i<nstat*ntime; ++i) {
    u[i] = 1e-4 * i;
    v[i] = 1e-4 * i;
    w[i] = 1e-9 * i;
  }
  // Determine the uv-range taking all baselines,times into account.
  double umin = -1e10;
  double vmin = -1e10;
  double umax = 1e10;
  double vmax = 1e10;
  // Loop over all times.
  for (int it=0; it<ntime; ++it) {
    const double* us = &(u[0]) + it*nstat;
    const double* vs = &(v[0]) + it*nstat;
    // Calculate u,v for baseline.
    for (int is2=0; is2<nstat; ++is2) {
      for (int is1=0; is1<is2; ++is1) {
	double uval = us[is2] - u[is1];
	if (uval < umin) {
	  umin = uval;
	}
	if (uval > umax) {
	  umax = uval;
	}
	double vval = vs[is2] - v[is1];
	if (vval < vmin) {
	  vmin = vval;
	}
	if (vval > vmax) {
	  vmax = vval;
	}
      }
    }
  }
  // Determine step; take some room for rounding problems.
  umin -= 0.0001;
  umax += 0.0001;
  vmin -= 0.0001;
  vmax += 0.0001;
  double ustep = (umax-umin) / (nu-1);
  double vstep = (vmax-vmin) / (nv-1);

  UVBrick uvbricks[nbrick];
  fillBricks (uvbricks);
  // Determine for each UV-brick which freq channels it contains.
  int uvbsch[nbrick];
  int uvbech[nbrick];
  // Find first brick containing f0.
  int ib = 0;
  for (; ib<nbrick; ++ib) {
    if (f0 < uvbricks[ib].freq) {
      break;
    }
    uvbsch[ib] = 0;
    uvbech[ib] = 0;
  }
  ASSERT (ib == 1);
  double freq = f0;
  for (int ic=0; ic<=nfreq; ++ic) {
    if (freq >= uvbricks[ib].freq) {
      uvbech[ib-1] = ic;
      uvbsch[ib] = ic;
      uvbech[ib] = ic;
      ++ib;
    }
    freq += df;
  }
  ASSERT (ib == nbrick);
  for (int i=ib; i<nbrick; ++ib) {
    uvbsch[i] = 0;
    uvbech[i] = 0;
  }

  // Calculate l,m,n of source.
  const double lk = ::cos(decs) * ::sin(ras-ra0);
  const double mk = ::sin(decs) * ::cos(dec0) - 
                    ::cos(decs) * ::sin(dec0) * ::cos(ras-ra0);
  const double nk = 1 - sqrt(lk*lk + mk*mk);
  const double wavel0 = C::_2pi * f0 / C::c;
  const double dwavel = df / f0;
  vector<CalcType> sf0(nstat);
  vector<CalcType> sdf(nstat);
  NSTimer timer  ("total         ");
  NSTimer timerl ("inner shift   ");
  NSTimer timers ("subtract      ");
  NSTimer timersl("outer subtract");
  NSTimer timers2("inner subtract");
  timer.start();

  // Array to hold per brick the visibility interpolated in u,v.
  VisType visxx[nbrick];
  VisType visxy[nbrick];
  VisType visyx[nbrick];
  VisType visyy[nbrick];

  // Loop over all times.
  for (int it=0; it<ntime; ++it) {
    const double* us = &(u[0]) + it*nstat;
    const double* vs = &(v[0]) + it*nstat;
    const double* ws = &(w[0]) + it*nstat;
    // Calculate phase shift terms per station for f0 and df.
    for (int is=0; is<nstat; ++is) {
      double r1 = (us[is]*lk + vs[is]*mk +  ws[is]*nk) * wavel0;
      sf0[is] = MAKECALCTYPE(cos(r1), sin(r1));
      r1 *= dwavel;
      sdf[is] = MAKECALCTYPE(cos(r1), sin(r1));
    }
    // Calculate phase shift per baseline and frequency.
    for (int is2=0; is2<nstat; ++is2) {
      for (int is1=0; is1<is2; ++is1) {
	CalcType val0 = sf0[is2] * conj(sf0[is1]);
	CalcType dval = sdf[is2] * conj(sdf[is1]);
	VisType* datap = data;
	timerl.start();
	for (int ifr=0; ifr<nfreq; ++ifr) {
	  val0 *= dval;
	  // Shift the data.
	  for (int ic=0; ic<ncorr; ++ic) {
	    *datap++ *= val0;
	  }
	}
	timerl.stop();

	// Subtract the UV-brick (FFT-ed image patch).
	timers.start();
	// Interpolate bi-linearly in u,v for each UV-brick.
	double uval = us[is2]-us[is1] - umin;
	double vval = vs[is2]-vs[is1] - vmin;
	int ubin = int(uval / ustep);
	int vbin = int(vval / vstep);
	double ud = uval - ubin*ustep;
	double vd = vval - vbin*vstep;
	for (int ib=0; ib<nbrick; ++ib) {
	  visxx[ib] = (1-ud)*(1-vd)*uvbricks[ib].visxx[ubin+vbin*nu] +
	    ud*(1-vd)*uvbricks[ib].visxx[ubin+1+vbin*nu] +
	    ud*vd*uvbricks[ib].visxx[ubin+(vbin+1)*nu] +
	    (1-ud)*vd*uvbricks[ib].visxx[ubin+1+(vbin+1)*nu];
	  visxy[ib] = (1-ud)*(1-vd)*uvbricks[ib].visxy[ubin+vbin*nu] +
	    ud*(1-vd)*uvbricks[ib].visxy[ubin+1+vbin*nu] +
	    ud*vd*uvbricks[ib].visxy[ubin+(vbin+1)*nu] +
	    (1-ud)*vd*uvbricks[ib].visxy[ubin+1+(vbin+1)*nu];
	  visyx[ib] = (1-ud)*(1-vd)*uvbricks[ib].visyx[ubin+vbin*nu] +
	    ud*(1-vd)*uvbricks[ib].visyx[ubin+1+vbin*nu] +
	    ud*vd*uvbricks[ib].visyx[ubin+(vbin+1)*nu] +
	    (1-ud)*vd*uvbricks[ib].visyx[ubin+1+(vbin+1)*nu];
	  visyy[ib] = (1-ud)*(1-vd)*uvbricks[ib].visyy[ubin+vbin*nu] +
	    ud*(1-vd)*uvbricks[ib].visyy[ubin+1+vbin*nu] +
	    ud*vd*uvbricks[ib].visyy[ubin+(vbin+1)*nu] +
	    (1-ud)*vd*uvbricks[ib].visyy[ubin+1+(vbin+1)*nu];
	}
	timersl.start();
	// Interpolate the UV-brick result in frequency and subtract.
	datap = data;
	for (int ib=0; ib<nbrick-1; ++ib) {
	  double stf = uvbricks[ib].freq;
	  double diff = uvbricks[ib+1].freq - stf;
	  CalcType slopexx = (visxx[ib+1] - visxx[ib]) / diff;
	  CalcType slopexy = (visxy[ib+1] - visxy[ib]) / diff;
	  CalcType slopeyx = (visyx[ib+1] - visyx[ib]) / diff;
	  CalcType slopeyy = (visyy[ib+1] - visyy[ib]) / diff;
	  VisType stxx = (f0 + uvbsch[ib]*df - stf) * slopexx;
	  VisType stxy = (f0 + uvbsch[ib]*df - stf) * slopexy;
	  VisType styx = (f0 + uvbsch[ib]*df - stf) * slopeyx;
	  VisType styy = (f0 + uvbsch[ib]*df - stf) * slopeyy;
	  VisType dxx = slopexx * df;
	  VisType dxy = slopexy * df;
	  VisType dyx = slopeyx * df;
	  VisType dyy = slopeyy * df;
	  timers2.start();
	  for (int ic=uvbsch[ib]; ic<uvbech[ib]; ++ic) {
	    *datap++ -= stxx;
	    stxx += dxx;
	    *datap++ -= stxy;
	    stxy += dxy;
	    *datap++ -= styx;
	    styx += dyx;
	    *datap++ -= styy;
	    styy += dyy;
	  }
	  timers2.stop();
	}
	timersl.stop();
	timers.stop();
      }
    }
  }
  timer.stop();
  timerl.print (cout);
  timers2.print (cout);
  timersl.print (cout);
  timers.print (cout);
  timer.print (cout);
}

double trycos (double b)
{
  NSTimer tim;
  tim.start();
  double a=0;
  for (int i=0; i<10000; ++i) {
    a += ::cos(b);
    b += 0.1;
  }
  tim.stop();
  tim.print(cout);
  return a;
}

int main ()
{
  trycos (2);
  INIT_LOGGER("tShift");
  try {
    // Do a shift.
    {
      doShift();
    }

  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
