#ifndef LOFAR_PL_TEST_C
#define LOFAR_PL_TEST_C

#include "A.h"
#include <PL/PLfwd.h>
#include <dtl/BoundIO.h>
#include <string>
#include <complex>
#include <iostream>

using std::string;
using dtl::blob;

class C : public A
{
public:
  C() : itsString("class C") {}
  C(const A& a, const blob& b, const string& s) :
    A(a), itsBlob(b), itsString(s)
  {}
  friend std::ostream& operator<<(std::ostream& os, const C& c);
private:
  friend class LOFAR::PL::TPersistentObject<C>;
  blob         itsBlob;
  string       itsString;
};

inline std::ostream& operator<<(std::ostream& os, const C& c) 
{
  using std::endl;
  os << endl << "C::A = " << (A&)(c);
  os << endl << "C.itsBlob    =" << std::hex ;
  for(uint i = 0; i < c.itsBlob.size(); i++) {
    os << " 0x" << (int)(c.itsBlob[i]); // << (i%10 == 0 ? "\n" : " ");
  }
  os << std::dec;
  os << endl << "C.itsString  = " << c.itsString;
  return os;
}
    
#endif
