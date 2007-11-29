#ifndef LOFAR_PL_TEST_B
#define LOFAR_PL_TEST_B

#include <PL/PLfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{

  class B
  {
  public:
    B() : 
      itsBool(false), itsShort(0), itsFloat(0.0), itsString("class B") 
    {}
    B(bool b, short i, float f, const string& s) :
      itsBool(b), itsShort(i), itsFloat(f), itsString(s)
    {}
    friend ostream& operator<<(ostream& os, const B& b);
  private:
    friend class LOFAR::PL::TPersistentObject<B>;
    bool         itsBool;
    short        itsShort;
    float        itsFloat;
    string       itsString;
  };

  inline ostream& operator<<(ostream& os, const B& b)
  {
    os << endl << "B.itsBool    = " << b.itsBool
       << endl << "B.itsShort   = " << b.itsShort
       << endl << "B.itsFloat   = " << b.itsFloat
       << endl << "B.itsString  = " << b.itsString;
    return os;
  }

} // namespace LOFAR

#endif
