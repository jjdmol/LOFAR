#include <fftw3.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/BasicSL/Complex.h>
#include <casa/OS/Timer.h>

using namespace casa;

#include <vector>
class X
{
public:
  X() {cout << "def ctor  " << this << endl;}
  X(const X& that)
  { cout<<"copy ctor "<<&that<<" to "<<this<<endl;}
  X& operator=(const X& that)
  { cout<<"assign    "<<&that<<" to "<<this<<endl; return *this;}
};
void testvec()
{
  std::vector<X> v;
  v.push_back (X());
  v.reserve(2);
  v.push_back (X());
  v.push_back (X());
  v.push_back (X());
  v.resize (20);
  std::vector<X> v2(v);
  std::vector<std::vector<X> > vv;
  vv.push_back (v);
  vv.resize (10);
}


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

void moveInx(Complex* to, Complex* fr, Complex* p0, int size)
{
  *to = *fr;
  //  cout << "move "<<(fr-p0)/size << ','<< (fr-p0)%size << " to "
  //       <<(to-p0)/size << ','<< (to-p0)%size << "   "<<*fr<<endl;
}

// It works fine.
void flipOdd (Matrix<Complex>& rData, bool toZero)
{
  int size = rData.shape()[0];
  int hsz = size/2;
  int lhsz = hsz;
  int rhsz = hsz;
  // Save the middle row and column.
  Vector<Complex> tmprow(size);
  Vector<Complex> tmpcol(size);
  objcopy (tmprow.data(), rData.data() + hsz*size, size);
  objcopy (tmpcol.data(), rData.data() + hsz, size, 1, size);
  std::complex<float>* __restrict__ p1f;
  std::complex<float>* __restrict__ p1t;
  std::complex<float>* __restrict__ p2f;
  std::complex<float>* __restrict__ p2t;
  int incr = size;
  int outm = size-1;
  // Determine where to start moving elements around.
  // Move to the middle line first, because that one is saved.
  if (toZero) {
    p1f = rData.data() + size*size - hsz;
    p1t = rData.data() + hsz*size + 1;
    p2f = rData.data() + hsz*size - size;
    p2t = p1f;
    incr = -size;
    outm = 0;
    rhsz++;
  } else {
    p1f = rData.data();
    p1t = rData.data() + hsz*size + hsz;
    p2f = p1t + size + 1;
    p2t = rData.data();
    lhsz++;
  }
  // Exchange q1 and q4.
  for (int j=0; j<hsz; ++j) {
    for (int i=0; i<hsz; ++i) {
      moveInx (p1t+i, p1f+i, rData.data(), size);
    }
    for (int i=0; i<hsz; ++i) {
      moveInx (p2t+i, p2f+i, rData.data(), size);
    }
    p1f += incr;
    p1t += incr;
    p2f += incr;
    p2t += incr;
  }
  if (toZero) {
    p1f = rData.data() + size*size - size;
    p1t = rData.data() + hsz*size - hsz + size;
    p2f = rData.data() + hsz*size - hsz;
    p2t = rData.data() + size*size - size + 1;
  } else {
    p1f = rData.data() + hsz + 1;
    p1t = rData.data() + hsz*size;
    p2f = p1t + size;
    p2t = rData.data() + hsz;
  }
  // Exchange q2 and a3.
  for (int j=0; j<hsz; ++j) {
    for (int i=0; i<hsz; ++i) {
      moveInx (p1t+i, p1f+i, rData.data(), size);
    }
    for (int i=0; i<hsz; ++i) {
      moveInx (p2t+i, p2f+i, rData.data(), size);
    }
    p1f += incr;
    p1t += incr;
    p2f += incr;
    p2t += incr;
  }
  // Put back the middle row and column and exchange top and bottom.
  objcopy (rData.data() + outm*size + rhsz, tmprow.data(), lhsz);
  objcopy (rData.data() + outm*size, tmprow.data() + lhsz, rhsz);
  objcopy (rData.data() + outm + rhsz*size, tmpcol.data(), lhsz, size, 1);
  objcopy (rData.data() + outm, tmpcol.data() + lhsz, rhsz, size, 1);
  return;
}

void oldFlip(Array<Complex>& cData, bool toZero)
{
  const IPosition shape = cData.shape();
  const uInt ndim = shape.nelements();
  const uInt nElements = cData.nelements();
  if (nElements == 1) {
    return;
  }
  AlwaysAssert(nElements != 0, AipsError);
  Block<Complex> buf;
  {
    Int buffLen = buf.nelements();
    for (uInt i = 0; i < ndim; ++i) {
      buffLen = max(buffLen, shape(i));
    }
    buf.resize(buffLen, False, False);
  }
  Bool dataIsAcopy;
  Complex * dataPtr = cData.getStorage(dataIsAcopy);
  Complex * buffPtr = buf.storage();
  Complex * rowPtr = 0;
  Complex * rowPtr2 = 0;
  Complex * rowPtr2o = 0;
  uInt rowLen, rowLen2, rowLen2o;
  uInt nFlips;
  uInt stride = 1;
  uInt r;
  uInt n=0;
  for (; n < ndim; ++n) {
    rowLen = shape(n);
    if (rowLen > 1) {
      rowLen2 = rowLen/2;
      rowLen2o = (rowLen+1)/2;
      nFlips = nElements/rowLen;
      rowPtr = dataPtr;
      r = 0;
      while (r < nFlips) {
        rowPtr2 = rowPtr + stride * rowLen2;
        rowPtr2o = rowPtr + stride * rowLen2o;
        if (toZero) {
          objcopy(buffPtr, rowPtr2, rowLen2o, 1u, stride);
          objcopy(rowPtr2o, rowPtr, rowLen2, stride, stride);
          objcopy(rowPtr, buffPtr, rowLen2o, stride, 1u);
        } else {
          objcopy(buffPtr, rowPtr, rowLen2o, 1u, stride);
          objcopy(rowPtr, rowPtr2o, rowLen2, stride, stride);
          objcopy(rowPtr2, buffPtr, rowLen2o, stride, 1u);
        }
        r++;
        rowPtr++;
        if (r%stride == 0) {
          rowPtr += stride*(rowLen-1);
        }
      }
      stride *= rowLen;
    }
  }
  cData.putStorage(dataPtr, dataIsAcopy);
}


void init (Array<Complex>& arr)
{
  arr = Complex(1,1);
  arr(arr.shape()/2) = Complex(0.5,0.5);
  arr(arr.shape()/4) = Complex(0.25,0.25);
  arr(arr.shape()/4*3) = Complex(0.75,0.75);
  //  indgen (arr, Complex(0.1,1.5), Complex(0.8/arr.size(), 0.9/arr.size()));
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
#pragma omp critical(tfftw_testfftw_destroy)
  {
    fftwf_destroy_plan (plan);
  }
  Array<Complex> res;
  res = arr;
  fftw_free (ptr);
  return res;
}

Array<Complex> testnoflip (int direction, int sz=128)
{
  Complex* ptr = static_cast<Complex*>(fftw_malloc ((sz*sz+1)*sizeof(Complex)));
  Matrix<Complex> arr(IPosition(2,sz,sz), ptr, SHARE);
  init (arr);
  Timer timer;
  fftwf_plan plan;
  plan = fftwf_plan_dft_2d(sz, sz,
                           reinterpret_cast<fftwf_complex*>(arr.data()),
                           reinterpret_cast<fftwf_complex*>(arr.data()),
                           direction, FFTW_ESTIMATE);
  fftwf_execute (plan);
  fftwf_destroy_plan (plan);
  preflip (arr);
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

void checkFlip (int sz=8)
{
  cout << "checkFlip" << endl;
  Matrix<Complex> mat1(sz,sz);
  Matrix<Complex> mat2, mat3;
  indgen (mat1, Complex(1,1), Complex(1,1));
  mat2 = mat1;
  mat3 = mat1;
  oldFlip (mat2, false);
  cout<<mat2<<mat3;
  flipOdd (mat1, false);
  cout << mat1;
  AlwaysAssertExit (allEQ(mat1, mat2));
  oldFlip (mat2, true);
  flipOdd (mat1, true);
  AlwaysAssertExit (allEQ(mat2, mat3));
  cout << mat1;
  AlwaysAssertExit (allEQ(mat1, mat3));
}

void timeFlip(int sz)
{
  sz = sz/2*2+1;   // make odd
  cout << "timeFlip " <<sz << endl;
  Matrix<Complex> mat1(sz,sz);
  Matrix<Complex> mat2;
  indgen (mat1, Complex(1,1), Complex(1,1));
  mat2 = mat1;
  Timer timer;
  flipOdd (mat1, true);
  timer.show ("flipodd");
  timer.mark();
  oldFlip (mat2, true);
  timer.show ("oldflip");
}


int main (int argc, char* [])
{
  ///  testvec();
  checkFlip(5);
  timeFlip (2048);
  // flipodd 3.5x faster than FFTServer::flip for sz=2049
  // flipodd 8.5x faster than FFTServer::flip for sz=2048(power of 2 issue?)
  {
    Array<Complex> arr1 = testfftw(FFTW_FORWARD, 8);
    Array<Complex> arr2 = testnoflip(FFTW_FORWARD, 8);
    AlwaysAssertExit (allNear(arr1,arr2,1e-5));
  }
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
  cout << "check parallel fftw 8,10,12,..,50" << endl;
#pragma omp parallel for
  for (int i=0; i<25; ++i) {
    cout<<"size: "<<8+i*2<<endl;
    Array<Complex> farrfftw = testfftw(FFTW_FORWARD, 8+i*2);
    Array<Complex> barrfftw = testfftw(FFTW_BACKWARD, 8+i*2);
    //cout<<farrfftw-fresults[i]<<endl;
    AlwaysAssertExit (allNearAbs(farrfftw, fresults[i], 1e-4));
    AlwaysAssertExit (allNearAbs(barrfftw, bresults[i], 1e-4));
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
