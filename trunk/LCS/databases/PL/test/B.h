#ifndef LOFAR_PL_TEST_B
#define LOFAR_PL_TEST_B

#include <Common/fwd/PL.h>
#include <string>
#include <iostream>

using std::string;

class B
{
public:
  B() : 
    itsBool(false), itsShort(0), itsFloat(0.0), itsString("class B") 
  {}
  B(bool b, short i, float f, const std::string& s) :
    itsBool(b), itsShort(i), itsFloat(f), itsString(s)
  {}
  friend std::ostream& operator<<(std::ostream& os, const B& b);
private:
  friend class TPersistentObject<B>;
  bool         itsBool;
  short        itsShort;
  float        itsFloat;
  string       itsString;
};

inline std::ostream& operator<<(std::ostream& os, const B& b)
{
  os << std::endl << "B.itsBool    = " << b.itsBool
     << std::endl << "B.itsShort   = " << b.itsShort
     << std::endl << "B.itsFloat   = " << b.itsFloat
     << std::endl << "B.itsString  = " << b.itsString;
  return os;
}

#endif
