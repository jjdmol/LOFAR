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
#include <Common/LofarLogger.h>
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

void testforwardnorm(FFTCMatrix& fftmat, Array<Complex>& arr,
                     const Array<Complex>& exp, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.normalized_forward (arr.shape()[0], arr.data());
  if (show) timer.show ("fornorm");
  if (exp.size() > 0) {
    AlwaysAssertExit (allNear(arr*Float(arr.size()), exp, 1e-5));
    cout << "  forward done"<<endl;
  }
}

void testbackwardnorm(FFTCMatrix& fftmat, Array<Complex>& arr,
                      const Array<Complex>& exp, bool show=false)
{
  init (arr);
  Timer timer;
  fftmat.normalized_backward (arr.shape()[0], arr.data());
  if (show) timer.show ("bacnorm");
  if (exp.size() > 0) {
    AlwaysAssertExit (allNear(arr/Float(arr.size()), exp, 1e-5));
    cout << "  backward done"<<endl;
  }
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

void checknorm(FFTCMatrix& fftmat, int sz)
{
  Matrix<Complex> arr(sz,sz);
  Array<Complex> resf = testfftw(fftmat, FFTW_FORWARD, sz);
  testforwardnorm (fftmat, arr, resf);
  Array<Complex> resb = testfftw(fftmat, FFTW_BACKWARD, sz);
  testbackwardnorm (fftmat, arr, resb);
}

void testOptSize()
{
  ASSERT (FFTCMatrix::optimalOddFFTSize(1) == 1);
  ASSERT (FFTCMatrix::optimalOddFFTSize(2) == 3);
  ASSERT (FFTCMatrix::optimalOddFFTSize(3) == 3);
  ASSERT (FFTCMatrix::optimalOddFFTSize(4) == 5);
  ASSERT (FFTCMatrix::optimalOddFFTSize(5) == 5);
  ASSERT (FFTCMatrix::optimalOddFFTSize(1400) == 1485);
  ASSERT (FFTCMatrix::optimalOddFFTSize(1485) == 1485);
  ASSERT (FFTCMatrix::optimalOddFFTSize(1485000) == 1485001);
  ASSERT (FFTCMatrix::optimalOddFFTSize(1485001) == 1485001);
}

int main (int argc, char*[])
{
  testOptSize();
  // Parallellize fftw.
  vector<FFTCMatrix> fftmats(OpenMP::maxThreads()); 
  vector<Array<Complex> > fresults;
  vector<Array<Complex> > bresults;
  // Make sure each element has its own Array object that do not share the count.
  // When doing e.g.
  //      vector<Array<Complex> > fresults(25);
  // it creates a temporary Array which gets copied to all elements, thus all
  // refer to the same underlying CountedPtr. Gives races conditions in the
  // parallel assign (which uses reference under water) below.
  fresults.reserve (25);
  bresults.reserve (25);
  for (int i=0; i<25; ++i) {
    fresults.push_back (Array<Complex>());
    bresults.push_back (Array<Complex>());
  }
  cout << "run parallel fftw 8,10,12,..,50 using "
       << fftmats.size() << " threads" << endl;
#pragma omp parallel 
  {
    FFTCMatrix fftm;
#pragma omp for
    for (int i=0; i<25; ++i) {
      fresults[i] = testfftw(fftm, FFTW_FORWARD, 8+i*2);
      bresults[i] = testfftw(fftm, FFTW_BACKWARD, 8+i*2);
    }
  }
  return 0;
  cout << "check serial fftw and casa 8,10,12,..,50" << endl;
  FFTCMatrix fftmat;
  Matrix<Complex> arr(8,8,Complex(1,0));
  fftmat.forward (8, arr.data());
  cout << arr;
  fftmat.backward (8, arr.data());
  cout << arr;
  fftmat.normalized_forward (8, arr.data());
  cout << arr;
  fftmat.normalized_backward (8, arr.data());
  cout << arr;
  for (uInt i=0; i<25; ++i) {
    cout<<"size: "<<8+i*2<<endl;
    Array<Complex> farrf = testfftw(fftmat, FFTW_FORWARD, 8+i*2);
    Array<Complex> barrf = testfftw(fftmat, FFTW_BACKWARD, 8+i*2);
    AlwaysAssertExit (allNear(farrf, fresults[i], 1e-3));
    AlwaysAssertExit (allNear(barrf, bresults[i], 1e-3));
    //    Array<Complex> farrc = testcasa(FFTW_FORWARD, 8+i*2);
    //Array<Complex> barrc = testcasa(FFTW_BACKWARD, 8+i*2);
    //AlwaysAssertExit (allNear(farrc, fresults[i], 1e-3));
    //AlwaysAssertExit (allNear(barrc, bresults[i], 1e-3));
    testforward(fftmat, farrf);
    testbackward(fftmat, barrf);
    AlwaysAssertExit (allNear(farrf, fresults[i], 1e-4));
    AlwaysAssertExit (allNear(barrf, bresults[i], 1e-4));
  }
  cout << "check normalized" << endl;
  checknorm(fftmat, 8);
  if (argc > 1) {
    cout << endl << "time forward fftw" << endl;
    testfftw(fftmat, FFTW_FORWARD, 4096, true);
    testcasa(FFTW_FORWARD, 4096, true);
    testfftw(fftmat, FFTW_FORWARD, 2187*2, true);  // is 2 * 3^7
    testcasa(FFTW_FORWARD, 2187*2, true);
    cout << endl << "time backward fftw" << endl;
    testfftw(fftmat, FFTW_BACKWARD, 4096, true);
    testcasa(FFTW_BACKWARD, 4096, true);
    testfftw(fftmat, FFTW_BACKWARD, 2187*2, true);
    testcasa(FFTW_BACKWARD, 2187*2, true);
    {
      Matrix<Complex> exp;
      Matrix<Complex> arr(2187*2, 2187*2);
      testforward(fftmat, arr, true);
      testbackward(fftmat, arr, true);
      testforwardnorm(fftmat, arr, exp, true);
      testbackwardnorm(fftmat, arr, exp, true);
    }
    int sizes[] = {66,65,64,63};
    for (uInt i=0; i<sizeof(sizes)/sizeof(int); ++i) {
      cout << "Test size " << 64*sizes[i] << endl;
      Matrix<Complex> arr(64*sizes[i], 64*sizes[i]);
      testforward(fftmat, arr, true);
      // Do it once more, so the planning time is nil.
      testforward(fftmat, arr, true);
    }
  }
  return 0;
}
