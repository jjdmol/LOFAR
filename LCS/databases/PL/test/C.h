#ifndef LOFAR_PL_TEST_C
#define LOFAR_PL_TEST_C

#include "A.h"
#include <PL/PLfwd.h>
#include <dtl/BoundIO.h>
#include <Common/lofar_string.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{

  using dtl::blob;

  class C : public A
  {
  public:
    C() : itsString("class C") {}
    C(const A& a, const blob& b, const string& s) :
      A(a), itsBlob(b), itsString(s)
    {}
    friend ostream& operator<<(ostream& os, const C& c);
  private:
    friend class LOFAR::PL::TPersistentObject<C>;
    blob         itsBlob;
    string       itsString;
  };

  inline ostream& operator<<(ostream& os, const C& c) 
  {
    os << endl << "C::A = " << (A&)(c);
    os << endl << "C.itsBlob    =" << hex ;
    for(uint i = 0; i < c.itsBlob.size(); i++) {
      os << " 0x" << (int)(c.itsBlob[i]); // << (i%10 == 0 ? "\n" : " ");
    }
    os << dec;
    os << endl << "C.itsString  = " << c.itsString;
    return os;
  }

} // namespace LOFAR
    
#endif
