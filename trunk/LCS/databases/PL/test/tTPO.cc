#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <iostream>
#include <pwd.h>

using namespace std;
using namespace dtl;

string getUserName()
{
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) == 0)
    return "";
  else
    return aPwd->pw_name;
}

class X
{
};

class Y : public X
{
};

namespace LOFAR { 

  namespace PL {

    template<> struct DBRep<X> 
    {
      ObjectId::oid_t  itsOid;
      ObjectId::oid_t  itsOwnerOid;


      ObjectId::oid_t getOid() const
      { return itsOid; }
    };

    template<>
    void BCA<X>::operator()(BoundIOs& cols, DataObj& rowbuf) 
    {
	  cols["ObjID"]  == rowbuf.itsOid;
	  cols["Owner"]  == rowbuf.itsOwnerOid;
    }
    
    template<>
    void TPersistentObject<X>::fromDBRep(const DBRep<X>& org)
    {
      metaData().oid()->set(org.itsOid);
      metaData().ownerOid()->set(org.itsOwnerOid);
    }

    template<>
    void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const
    {
      dest.itsOid   = metaData().oid()->get();
      dest.itsOwnerOid = metaData().ownerOid()->get();
    }

    template<> void TPersistentObject<X>::init() 
    { 
      metaData().tableName() = "X"; 
    }
    
    template<> void TPersistentObject<X>::initAttribMap()
    {
      theirAttribMap["world"] = "World";
    }

    template<> struct DBRep<Y> 
    {
      ObjectId::oid_t  itsOid;
      ObjectId::oid_t  itsOwnerOid;


      ObjectId::oid_t getOid() const
      { return itsOid; }
    };

    template<>
    void BCA<Y>::operator()(BoundIOs& cols, DataObj& rowbuf) 
    {
	  cols["ObjID"]  == rowbuf.itsOid;
	  cols["Owner"]  == rowbuf.itsOwnerOid;
    }
    
    template<>
    void TPersistentObject<Y>::fromDBRep(const DBRep<Y>& org)
    {
      metaData().oid()->set(org.itsOid);
      metaData().ownerOid()->set(org.itsOwnerOid);
    }

    template<>
    void TPersistentObject<Y>::toDBRep(DBRep<Y>& dest) const
    {
      dest.itsOid   = metaData().oid()->get();
      dest.itsOwnerOid = metaData().ownerOid()->get();
    }

    template<> void TPersistentObject<Y>::initAttribMap()
    {
      theirAttribMap["hello"] = "@Hello";
    }

    template<> void TPersistentObject<Y>::init() 
    {
      Pointer p(new TPersistentObject<X>(*itsObjectPtr));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      tableName("Y");
      initAttribMap();
    }

//     template<> TPersistentObject<Y>::attribmap_t theirAttribMap;

  }

}

using namespace LOFAR::PL;
using namespace LOFAR;

int main()
{
  Y y;
  ObjectId oid;
  QueryObject q;
  PersistenceBroker* b = new PersistenceBroker();

  try {
    cout << "Try to connect to database ...";
    b->connect(getUserName(),"postgres","");
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  TPersistentObject<Y> tpoy(y);
  try {
    cout << "tpoy.attrib(\"hello.world\") = " 
         << attrib<Y>("hello.world") << endl;
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  return 0;

  try {
    cout << "Trying save() ...";
    b->save(tpoy);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  oid.set(tpoy.metaData().oid()->get());

  try {
    cout << "Forcing insert() ...";
    b->save(tpoy, PersistenceBroker::INSERT);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Forcing update() ...";
    b->save(tpoy, PersistenceBroker::UPDATE);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying erase() ...";
    b->erase(tpoy);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying retrieve(ObjectId&) ...";
    b->retrieve<Y>(oid);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying retrieve<Y>(q) ...";
    b->retrieve<Y>(q);
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  return 0;
}
