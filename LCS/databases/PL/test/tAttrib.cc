#include <PL/Attrib.h>
#include <PL/DBRepHolder.h>

using namespace LOFAR::PL;
using namespace dtl;

class X
{
};

namespace LOFAR { 

  namespace PL {

    template<> struct DBRep<X>
    {
      void bindCols(dtl::BoundIOs&) {}
    };
    
    template<> void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const
    {
    }

    template<> void TPersistentObject<X>::fromDBRep(const DBRep<X>& src)
    {
    }

    template<> void TPersistentObject<X>::init() 
    { 
      metaData().tableName() = "X"; 
    }

    template<> void TPersistentObject<X>::initAttribMap()
    {
      theirAttribMap["Hello"] = "World";
    }

  }
}
    

int main()
{
  try {
    TPersistentObject<X> tpox;
    TPersistentObject<X>::attribmap_t tpoxmap;
    TPersistentObject<X>::attribmap_t::const_iterator it;
    tpoxmap = tpox.attribMap();
    for (it = tpoxmap.begin(); it != tpoxmap.end(); ++it) {
      cout << "[\"" << it->first << "\"] = \"" << it->second << "\"" << endl;
    }
    cout << attrib<X>("Hello") << endl;
  }
  catch (LOFAR::Exception& e) {
    cerr << e << endl;
  }
  return 0;
}
