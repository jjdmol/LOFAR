#ifndef LOFAR_PL_TEST_A
#define LOFAR_PL_TEST_A

#include "B.h"
#include <Common/fwd/PL.h>
#include <string>
#include <complex>
#include <iostream>

using std::string;
using std::complex;

class A
{
public:
  A() : 
    itsInt(0), itsDouble(0.0), itsString("class A") {}
  A(int i, double d, const string& s, const complex<double>& c, 
    const B& b) :
    itsInt(i), itsDouble(d), itsString(s), itsComplex(c), itsB(b)
  {}
  friend std::ostream& operator<<(std::ostream& os, const A& a);
private:
  friend class    TPersistentObject<A>;
  int             itsInt;
  double          itsDouble;
  string          itsString;
  complex<double> itsComplex;
  B               itsB;
};

inline std::ostream& operator<<(std::ostream& os, const A& a) 
{
  os << std::endl << "A.itsInt     = " << a.itsInt
     << std::endl << "A.itsDouble  = " << a.itsDouble
     << std::endl << "A.itsString  = " << a.itsString
     << std::endl << "A.itsComplex = " << a.itsComplex
     << std::endl << "A.itsB       = " << a.itsB;
  return os;
}

#endif
