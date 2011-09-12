#include <fftw3.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/BasicSL/Complex.h>
#include <casa/OS/Timer.h>

using namespace casa;

// Flip the quadrants which is needed for the FFT.
//  q1 q2    gets   q4 q3
//  q3 q4           q2 q1
void flip(Matrix<Complex>& rData)
{
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%2==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0]/2;
  uInt h1 = shape[1]/2;
  // Use 2 separate loops to be more cache local.
  for (uInt j=0; j<h1; ++j) {
    for (uInt i=0; i<h0; ++i) {
      Complex tmp1 = rData(i,j);
      rData(i,j) = rData(i+h0,j+h1);
      rData(i+h0,j+h1) = tmp1;
    }
  }
  for (uInt j=0; j<h1; ++j) {
    for (uInt i=0; i<h0; ++i) {
      Complex tmp1 = rData(i+h0,j);
      rData(i+h0,j) = rData(i,j+h1);
      rData(i,j+h1) = tmp1;
    }
  }
}

void flipopt(Matrix<Complex>& rData)
{
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%2==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0]/2;
  uInt h1 = shape[1]/2;
  // Use 2 separate loops to be more cache local.
  Complex* p1 = rData.data();
  Complex* p2 = rData.data() + h1*shape[0] + h0;
  for (int k=0; k<2; ++k) {
    for (uInt j=0; j<h1; ++j) {
      for (uInt i=0; i<h0; ++i) {
        Complex tmp1 = *p1;
        *p1++ = *p2;
        *p2++ = tmp1;
      }
      p1 += h0;
      p2 += h0;
    }
    p1 = rData.data() + h0;
    p2 = rData.data() + h1*shape[0];
  }
}

void flipop2(Matrix<Complex>& rData)
{
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%2==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0]/2;
  uInt h1 = shape[1]/2;
  // Use 2 separate loops to be more cache local.
  Complex* q1 = rData.data();
  Complex* q3 = rData.data() + h1*shape[0];
  const Complex* endj = q3;
  for (; q1<endj; q1+=h0, q3+=h0) {
    Complex* q2 = q1 + h0;
    Complex* q4 = q3 + h0;
    const Complex* endi = q2;
    while (q1<endi) {
      Complex tmp1 = *q1;
      *q1++ = *q4;
      *q4++ = tmp1;
      Complex tmp2 = *q2;
      *q2++ = *q3;
      *q3++ = tmp2;
    }
  }
}

// The output flip can be avoided by negating every other input element.
// So do the flip and negation jointly (only for multiple of 4 elements).
void preflip(Matrix<Complex>& rData)
{
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%4==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0]/2;
  uInt h1 = shape[1]/2;
  // Use separate loops to be more cache local.
  for (uInt j=0; j<h1; j+=2) {
    for (uInt i=0; i<h0; i+=2) {
      Complex tmp1 = rData(i,j);
      rData(i,j) = rData(i+h0,j+h1);
      rData(i+h0,j+h1) = tmp1;
      Complex tmp2 = -rData(i+1,j);
      rData(i+1,j) = -rData(i+h0+1,j+h1);
      rData(i+h0+1,j+h1) = tmp2;
    }
  }
  for (uInt j=1; j<h1; j+=2) {
    for (uInt i=0; i<h0; i+=2) {
      Complex tmp1 = -rData(i,j);
      rData(i,j) = -rData(i+h0,j+h1);
      rData(i+h0,j+h1) = tmp1;
      Complex tmp2 = rData(i+1,j);
      rData(i+1,j) = rData(i+h0+1,j+h1);
      rData(i+h0+1,j+h1) = tmp2;
    }
  }
  for (uInt j=0; j<h1; j+=2) {
    for (uInt i=0; i<h0; i+=2) {
      Complex tmp1 = rData(i+h0,j);
      rData(i+h0,j) = rData(i,j+h1);
      rData(i,j+h1) = tmp1;
      Complex tmp2 = -rData(i+h0+1,j);
      rData(i+h0+1,j) = -rData(i+1,j+h1);
      rData(i+1,j+h1) = tmp2;
    }
  }
  for (uInt j=1; j<h1; j+=2) {
    for (uInt i=0; i<h0; i+=2) {
      Complex tmp1 = -rData(i+h0,j);
      rData(i+h0,j) = -rData(i,j+h1);
      rData(i,j+h1) = tmp1;
      Complex tmp2 = rData(i+h0+1,j);
      rData(i+h0+1,j) = rData(i+1,j+h1);
      rData(i+1,j+h1) = tmp2;
    }
  }
}

void scaleflip(Matrix<Complex>& rData)
{
  Float scale = float(1./rData.size());
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%2==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0]/2;
  uInt h1 = shape[1]/2;
  // Use 2 separate loops to be more cache local.
  for (uInt j=0; j<h1; ++j) {
    for (uInt i=0; i<h0; ++i) {
      Complex tmp1 = rData(i,j) * scale;
      rData(i,j) = rData(i+h0,j+h1) * scale;
      rData(i+h0,j+h1) = tmp1;
    }
  }
  for (uInt j=0; j<h1; ++j) {
    for (uInt i=0; i<h0; ++i) {
      Complex tmp1 = rData(i+h0,j) *scale;
      rData(i+h0,j) = rData(i,j+h1) * scale;
      rData(i,j+h1) = tmp1;
    }
  }
}

void premult(Matrix<Complex>& rData)
{
  const IPosition& shape = rData.shape();
  AlwaysAssert(shape[0]%2==0 && shape[1]%2==0, AipsError);
  uInt h0 = shape[0];
  uInt h1 = shape[1];
  uInt st = 1;
  for (uInt j=0; j<h1; ++j) {
    for (uInt i=st; i<h0; i+=2) {
      rData(i,j) *= float(-1);
    }
    st = (st==0  ?  1 : 0);
  }
}

void init (Array<Complex>& arr)
{
  //  indgen (arr, Complex(1.1,1.1), Complex(0.8/arr.size(), 0.8/arr.size()));
  arr = Complex(1,1);
  arr(arr.shape()/2) = Complex(0.5,0.5);
  arr(arr.shape()/4) = Complex(0.25,0.25);
  arr(arr.shape()/4*3) = Complex(0.75,0.75);
}

Array<Complex> testfftw(int direction, int sz=128, bool show=false, int align=0)
{
  if (show) cout <<"fftw size=" << sz << " dir=" << direction
                 << " align=" << align << endl;
  Complex* ptr = static_cast<Complex*>(fftw_malloc ((sz*sz+1)*sizeof(Complex)));
  Matrix<Complex> arr(IPosition(2,sz,sz), ptr+align, SHARE);
  if (show) cout << ptr << ' ' << arr.data() << endl;
  Timer timer;
  fftwf_plan plan;
#pragma omp critical(tfftw_testfftw)
  {
    plan = fftwf_plan_dft_2d(sz, sz,
                             reinterpret_cast<fftwf_complex*>(arr.data()),
                             reinterpret_cast<fftwf_complex*>(arr.data()),
                             direction, FFTW_ESTIMATE);
  }
  if (show) timer.show ("plan   ");
  init (arr);
  if (direction == FFTW_FORWARD) {
    if (sz%4 == 0) {
      timer.mark();
      preflip(arr);
      if (show) timer.show ("preflip");
      timer.mark();
      fftwf_execute (plan);
      if (show) timer.show ("exec   ");
    } else {
      timer.mark();
      flip(arr);
      if (show) timer.show ("flip1  ");
      timer.mark();
      fftwf_execute (plan);
      if (show) timer.show ("exec   ");
      timer.mark();
      flip (arr);
      if (show) timer.show ("flip2  ");
    }
  } else {
    timer.mark();
    flip(arr);
    if (show) timer.show ("flip1  ");
    timer.mark();
    fftwf_execute (plan);
    if (show) timer.show ("exec   ");
    timer.mark();
    scaleflip (arr);
    if (show) timer.show ("scalefl");
  }
  Array<Complex> res;
  res = arr;
  fftw_free (ptr);
  return res;
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

void testflip (int sz=4096)
{
  Matrix<Complex> arr1(sz,sz);
  Matrix<Complex> arr2(sz,sz);
  indgen(arr1);
  indgen(arr2);
  Timer timer;
  flip(arr1);
  timer.show ("flip   ");
  timer.mark();
  flipop2(arr2);
  timer.show ("flipop2");
  AlwaysAssertExit (allEQ(arr1, arr2));
}

int main (int argc)
{
  cout << "check serial fftw and casa 8,10,12,..,50" << endl;
  vector<Array<Complex> > fresults;
  vector<Array<Complex> > bresults;
  for (uInt i=0; i<25; ++i) {
    fresults.push_back (testfftw(FFTW_FORWARD, 8+i*2));
    bresults.push_back (testfftw(FFTW_BACKWARD, 8+i*2));
    Array<Complex> farr = testcasa(FFTW_FORWARD, 8+i*2);
    Array<Complex> barr = testcasa(FFTW_BACKWARD, 8+i*2);
    AlwaysAssertExit (allNear(farr, fresults[i], 1e-3));
    AlwaysAssertExit (allNear(barr, bresults[i], 1e-3));
  }
  // Parallellize fftw.
  cout << "check parallel fftw and casa 8,10,12,..,50" << endl;
#pragma omp parallel for
  for (uInt i=0; i<25; ++i) {
    Array<Complex> farrfftw = testfftw(FFTW_FORWARD, 8+i*2);
    Array<Complex> barrfftw = testfftw(FFTW_BACKWARD, 8+i*2);
    AlwaysAssertExit (allNear(farrfftw, fresults[i], 1e-5));
    AlwaysAssertExit (allNear(barrfftw, bresults[i], 1e-5));
  }
  if (argc > 1) {
    cout << endl << "time flip and flipopt" << endl;
    testflip(1*4096);
    testflip(2*4096);
    cout << endl << "time forward fftw" << endl;
    testfftw(FFTW_FORWARD, 4096, true);
    testfftw(FFTW_FORWARD, 4096, true, 1);
    testcasa(FFTW_FORWARD, 4096, true);
    testfftw(FFTW_FORWARD, 2187*2, true);  // is 3^7 * 2
    testfftw(FFTW_FORWARD, 2187*2, true, 1);
    testcasa(FFTW_FORWARD, 2187*2, true);
    cout << endl << "time backward fftw" << endl;
    testfftw(FFTW_BACKWARD, 4096, true);
    testcasa(FFTW_BACKWARD, 4096, true);
    testfftw(FFTW_BACKWARD, 2187*2, true);
    testcasa(FFTW_BACKWARD, 2187*2, true);
  }
  return 0;
}
