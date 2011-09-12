//# tFFTCMatrix.cc: Test program for class FFTCMatrix
//#
//# Copyright (C) 2011
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

#include <LofarFT/FFTCMatrix.h>
#include <Common/OpenMP.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/OS/Timer.h>

using namespace LOFAR;
using namespace casa;

void init (Array<Complex>& arr)
{
  //  indgen (arr, Complex(1.1,1.1), Complex(0.8/arr.size(), 0.8/arr.size()));
  arr = Complex(1,1);
  arr(arr.shape()/2) = Complex(0.5,0.5);
  arr(arr.shape()/4) = Complex(0.25,0.25);
  arr(arr.shape()/4*3) = Complex(0.75,0.75);
}

Array<Complex> testfftw(FFTCMatrix& fftmat, int direction,
                        int sz=128, bool show=false)
{
  if (show) cout <<"fftw size=" << sz << " dir=" << direction << endl;
  Timer timer;
  fftmat.plan (sz, direction==FFTW_FORWARD);
  if (show) timer.show ("plan   ");
  Array<Complex> arr(IPosition(2,fftmat.size(), fftmat.size()),
                     fftmat.data(), SHARE);
  init (arr);
  timer.mark();
  fftmat.fft();
  if (show) timer.show ("fft    ");
  Array<Complex> res;
  res = arr;
  return res;
}

void testforward(FFTCMatrix& fftmat, Array<Complex>& arr, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.forward (arr.shape()[0], arr.data());
  if (show) timer.show ("forward");
}

void testbackward(FFTCMatrix& fftmat, Array<Complex>& arr, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.backward (arr.shape()[0], arr.data());
  if (show) timer.show ("bacward");
}

void testforwardnorm(FFTCMatrix& fftmat, Array<Complex>& arr, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.normalized_forward (arr.shape()[0], arr.data());
  if (show) timer.show ("fornorm");
}

void testbackwardnorm(FFTCMatrix& fftmat, Array<Complex>& arr, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.normalized_backward (arr.shape()[0], arr.data());
  if (show) timer.show ("bacnorm");
}

Array<Complex> testcasa(int direction, int sz=128, bool show=false)
{
  cout <<"casa size=" << sz << " dir=" << direction << endl;
  Array<Complex> arr(IPosition(2,sz,sz));
  init (arr);
  Timer timer;
  FFTServer<Float,Complex> server(arr.shape());
  server.fft (arr, direction==FFTW_FORWARD);
  if (show) timer.show ("casa   ");
  return arr;
}


int main (int argc)
{
  FFTCMatrix fftmat;
  cout << "check serial fftw and casa 8,10,12,..,50" << endl;
  vector<Array<Complex> > fresults;
  vector<Array<Complex> > bresults;
  for (uInt i=0; i<25; ++i) {
    fresults.push_back (testfftw(fftmat, FFTW_FORWARD, 8+i*2, false));
    bresults.push_back (testfftw(fftmat, FFTW_BACKWARD, 8+i*2, false));
    Array<Complex> farr = testcasa(FFTW_FORWARD, 8+i*2);
    Array<Complex> barr = testcasa(FFTW_BACKWARD, 8+i*2);
    AlwaysAssertExit (allNear(farr, fresults[i], 1e-3));
    AlwaysAssertExit (allNear(barr, bresults[i], 1e-3));
    testforward(fftmat, farr);
    testbackward(fftmat, barr);
    AlwaysAssertExit (allNear(farr, fresults[i], 1e-4));
    AlwaysAssertExit (allNear(barr, bresults[i], 1e-4));
  }
  // Parallellize fftw.
  cout << "check parallel fftw and casa 8,10,12,..,50" << endl;
  vector<FFTCMatrix> fftmats(OpenMP::maxThreads()); 
#pragma omp parallel for
  for (uInt i=0; i<25; ++i) {
    int tnr = OpenMP::threadNum();
    Array<Complex> farrfftw = testfftw(fftmats[tnr], FFTW_FORWARD, 8+i*2);
    Array<Complex> barrfftw = testfftw(fftmats[tnr], FFTW_BACKWARD, 8+i*2);
    AlwaysAssertExit (allNear(farrfftw, fresults[i], 1e-5));
    AlwaysAssertExit (allNear(barrfftw, bresults[i], 1e-5));
  }
  if (argc > 1) {
    cout << endl << "time forward fftw" << endl;
    testfftw(fftmat, FFTW_FORWARD, 4096, true);
    testcasa(FFTW_FORWARD, 4096, true);
    testfftw(fftmat, FFTW_FORWARD, 2187*2, true);  // is 3^7 * 2
    testcasa(FFTW_FORWARD, 2187*2, true);
    cout << endl << "time backward fftw" << endl;
    testfftw(fftmat, FFTW_BACKWARD, 4096, true);
    testcasa(FFTW_BACKWARD, 4096, true);
    testfftw(fftmat, FFTW_BACKWARD, 2187*2, true);
    testcasa(FFTW_BACKWARD, 2187*2, true);
    Matrix<Complex> arr(2187*2, 2187*2);
    testforward(fftmat, arr, true);
    testbackward(fftmat, arr, true);
    testforwardnorm(fftmat, arr, true);
    testbackwardnorm(fftmat, arr, true);
  }
  return 0;
}
