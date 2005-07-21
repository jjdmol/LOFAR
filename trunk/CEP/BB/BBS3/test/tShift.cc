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
#include <Common/Timer.h>
#include <casa/BasicSL/Constants.h>
#include <vector>
#include <cmath>
#include <stdexcept>

#if defined __SSE__
#include <emmintrin.h>
#endif

using namespace LOFAR;
using namespace std;
using namespace casa;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

const int ncorr = 4;
const int nstat = 77;
const int ntime = 100;
const int nfreq = 64;
const double f0 = 137750000;
const double df = 500000;
const double ras = 2.734;
const double decs = 0.45379;
const double ra0 = 2.733;
const double dec0 = 0.453;

void doShift()
{
  cout << "ncorr=" << ncorr
       << ", nfreq=" << nfreq
       << ", nstat=" << nstat
       << " (nbl=" << nstat*(nstat-1)/2 << ")"
       << ", ntime=" << ntime
       << "; nvis=" << double(ncorr)*nfreq*ntime*nstat*(nstat-1)/2
       << endl;

  // Define some fake data.
  //vector<fcomplex> datavec(ncorr*nfreq+1);
  fcomplex datavec[ncorr*nfreq] __attribute__ ((aligned(16)));
  float out_r[ncorr*nfreq] __attribute__ ((aligned(16)));
  float out_i[ncorr*nfreq] __attribute__ ((aligned(16)));

  for (int i=0; i<ncorr*nfreq; ++i) {
    datavec[i] = makefcomplex(1e-3*i, 1e-4*i);
  }
  fcomplex* data = &(datavec[0]);

  // Arrays for U,V,W per station per time stamp.
  vector<double> u(nstat*ntime);
  vector<double> v(nstat*ntime);
  vector<double> w(nstat*ntime);
  for (int i=0; i<nstat*ntime; ++i) {
    u[i] = 1e-4 * i;
    v[i] = 1e-4 * i;
    w[i] = 1e-9 * i;
  }
  const double lk = ::cos(decs) * ::sin(ras-ra0);
  const double mk = ::sin(decs) * ::cos(dec0) - 
                    ::cos(decs) * ::sin(dec0) * ::cos(ras-ra0);
  const double nk = 1 - sqrt(lk*lk + mk*mk);
  const double wavel0 = C::_2pi * f0 / C::c;
  const double dwavel = df / f0;
  vector<fcomplex> sf0(nstat);
  vector<fcomplex> sdf(nstat);
  NSTimer timer, timerl;
  timer.start();

  // Loop over all times.
  for (int it=0; it<ntime; ++it) {
    const double* us = &(u[0]) + it*nstat;
    const double* vs = &(v[0]) + it*nstat;
    const double* ws = &(w[0]) + it*nstat;
    // Calculate phase shift terms per station for f0 and df.
    for (int is=0; is<nstat; ++is) {
      double r1 = (us[is]*lk + vs[is]*mk +  ws[is]*nk) * wavel0;
      sf0[is] = makefcomplex(cos(r1), sin(r1));
      r1 *= dwavel;
      sdf[is] = makefcomplex(cos(r1), sin(r1));
    }
    // Calculate phase shift per baseline and frequency.
    for (int is2=0; is2<nstat; ++is2) {
      for (int is1=0; is1<is2; ++is1) {
	fcomplex val0 = sf0[is2] * conj(sf0[is1]);
	fcomplex dval = sdf[is2] * conj(sdf[is1]);
	timerl.start();
	  // Shift the data.
#if defined __SSE__
	__m128 val0_r = _mm_set_ps1(real(val0));
	__m128 val0_i = _mm_set_ps1(imag(val0));
	__m128 dval_r = _mm_set_ps1(real(dval));
	__m128 dval_i = _mm_set_ps1(imag(dval));

	for (int ifr=0; ifr<nfreq; ++ifr) {
	  __m128 tmp = _mm_sub_ps(_mm_mul_ps(val0_r, dval_r), _mm_mul_ps(val0_i, dval_i));
	  val0_i = _mm_add_ps(_mm_mul_ps(val0_r, dval_i), _mm_mul_ps(val0_i, dval_r));
	  val0_r = tmp;

	  __m128 first  = ((__m128 *) (data + 4 * ifr)) [0];
	  __m128 second = ((__m128 *) (data + 4 * ifr)) [1];

	  __m128 data_r = _mm_shuffle_ps(first, second, _MM_SHUFFLE(2,0,2,0));
	  __m128 data_i = _mm_shuffle_ps(first, second, _MM_SHUFFLE(3,1,3,1));

	  ((__m128 *) out_r)[ifr] = _mm_sub_ps(_mm_mul_ps(data_r, val0_r), _mm_mul_ps(data_i, val0_i));
	  ((__m128 *) out_i)[ifr] = _mm_add_ps(_mm_mul_ps(data_r, val0_i), _mm_mul_ps(data_i, val0_r));
	}
#else
	for (int ifr=0; ifr<nfreq; ++ifr) {
	  val0 *= dval;
	  for (int ico = 0; ico < ncorr; ico ++) {
	    fcomplex val = data[ncorr*ifr+ico] * val0;
	    out_r[ncorr*ifr+ico] = real(val);
	    out_i[ncorr*ifr+ico] = imag(val);
	  }
	}
#endif
	timerl.stop();
      }
    }
  }
  timer.stop();
  for (int i = 0; i < 10; i ++)
    cout << '(' << out_r[i] << ',' << out_i[i] << ")\n";
  timerl.print (cout);
  timer.print (cout);
}


int main ()
{
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
