#ifndef LOFAR_PL_TEST_A
#define LOFAR_PL_TEST_A

#include "B.h"
#include <PL/PLfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{

  class A
  {
  public:
    A() : 
      itsInt(0), itsDouble(0.0), itsString("class A") {}
    A(int i, double d, const string& s, const complex<double>& c, 
      const B& b) :
      itsInt(i), itsDouble(d), itsString(s), itsComplex(c), itsB(b)
    {}
    friend ostream& operator<<(ostream& os, const A& a);
  private:
    friend class    LOFAR::PL::TPersistentObject<A>;
    int             itsInt;
    double          itsDouble;
    string          itsString;
    complex<double> itsComplex;
    B               itsB;
  };

  inline ostream& operator<<(ostream& os, const A& a) 
  {
    os << endl << "A.itsInt     = " << a.itsInt
       << endl << "A.itsDouble  = " << a.itsDouble
       << endl << "A.itsString  = " << a.itsString
       << endl << "A.itsComplex = " << a.itsComplex
       << endl << "A.itsB       = " << a.itsB;
    return os;
  }

} // namespace LOFAR

#endif
