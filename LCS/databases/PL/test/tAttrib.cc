#include <PL/Attrib.h>

using namespace LOFAR::PL;
using namespace dtl;

class X
{
};

namespace LOFAR { 

  namespace PL {

    template<> struct DBRep<X> : public DBRepMeta
    {
//       ObjectId::oid_t  itsOid;
//       ObjectId::oid_t  itsOwnerOid;
    };

    template<>
    void BCA<X>::operator()(BoundIOs& cols, DataObj& rowbuf) 
    {
// 	  cols["ObjID"]  == rowbuf.itsOid;
// 	  cols["Owner"]  == rowbuf.itsOwnerOid;
    }
    
    template<>
    void TPersistentObject<X>::fromDBRep(const DBRep<X>& org)
    {
//       metaData().oid()->set(org.itsOid);
//       metaData().ownerOid()->set(org.itsOwnerOid);
    }

    template<>
    void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const
    {
//       dest.itsOid   = metaData().oid()->get();
//       dest.itsOwnerOid = metaData().ownerOid()->get();
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
