#include <iostream>
#include <sstream>
#include <complex>
#include <Common/Timer.h>

using namespace LOFAR;
using namespace std;

// Generic Saxpy function
template<class T, class U>
void saxpy1(int n, const T* x, const U a, T* y)
{
   NSTimer tim;
   tim.start();
   for (int i=0; i<n; i++)
   {
     (*(y++))+=a*(*(x++));
   }
   tim.stop();
   cout<< tim;
}
template<class T, class U>
void saxpy2(int n, const T* x, const U a, T* y)
{
   NSTimer tim;
   tim.start();
   for (int i=0; i<n; i++)
   {
     y[i]+=a*x[i];
   }
   tim.stop();
   cout<< tim;
}

template<class T, class U>
void saxpy(int n, T val)
{
  T* xp = new T[n];
  T* yp = new T[n];
  fill (xp, xp+n, val);
  fill (yp, yp+n, val);
  U u = U();
  cout << xp << ' ' << yp << endl;
  saxpy1 (n, xp, u, yp);
  saxpy2 (n, xp, u, yp);
  delete xp;
  delete yp;
}

int main (int argc, const char* argv[])
{
  int n = 2048;
  if (argc > 1) {
    istringstream istr(argv[1]);
    istr >> n;
  }
  cout<< "float,float " << n << endl;
  saxpy<float,float> (n, 0);
  cout<< "__complex__ float,float " << n << endl;
  saxpy<__complex__ float,float> (n, 0.+0.i);
  cout<< "std::complex<float>,float " << n << endl;
  saxpy<std::complex<float>,float> (n, std::complex<float>());
  cout<< "double,double " << n << endl;
  saxpy<double,double> (n, 0);
  cout<< "__complex__ double,double " << n << endl;
  saxpy<__complex__ double,double> (n, 0.+0.i);
  cout<< "std::complex<double>,double " << n << endl;
  saxpy<std::complex<double>,double> (n, std::complex<double>());
  cout<< "std::complex<double>,std::complex<double> " << n << endl;
  saxpy<std::complex<double>,std::complex<double> > (n, std::complex<double>());
  return 0;
}
