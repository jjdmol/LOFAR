#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Query.h>
#include <PL/Attrib.h>
#include <Common/Debug.h>
#include <iostream>
#include <pwd.h>

using namespace std;
using namespace dtl;

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
      void bindCols(dtl::BoundIOs& cols) {}
    };

    template<> void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const
    {
    }

    template<> void TPersistentObject<X>::fromDBRep(const DBRep<X>& src)
    {
    }

    template<> void TPersistentObject<X>::initAttribMap()
    {
    }

    template<> void TPersistentObject<X>::init() 
    { 
      metaData().tableName() = "X"; 
    }
    
    template<> struct DBRep<Y>
    {
      void bindCols(dtl::BoundIOs& cols) {}
   };

    template<> void TPersistentObject<Y>::toDBRep(DBRep<Y>& dest) const
    {
    }

    template<> void TPersistentObject<Y>::fromDBRep(const DBRep<Y>& src)
    {
    }

    template<> void TPersistentObject<Y>::initAttribMap()
    {
      theirAttribMap["hello"] = "@Hello@";
    }

    template<> void TPersistentObject<Y>::init() 
    {
      Pointer p(new TPersistentObject<X>(data()));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      tableName("Y");
      initAttribMap();
    }

  }

}

using namespace LOFAR::PL;
using namespace LOFAR;

int main(int argc, const char* argv[])
{
  Debug::initLevels (argc, argv);

  Y y;
  ObjectId oid;
  QueryObject q;
  PersistenceBroker* b = new PersistenceBroker();

  try {
    cout << "Try to connect to database ...";
    b->connect("test","postgres","");
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  TPersistentObject<Y> tpoy(y);
  try {
    cout << "attrib<Y>(\"hello.world\") = " 
         << attrib<Y>("hello.world") << endl;
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  return 0;

  try {
    cout << "Trying save() ...";
    b->save(tpoy);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  oid.set(tpoy.metaData().oid()->get());

  try {
    cout << "Forcing insert() ...";
    b->save(tpoy, PersistenceBroker::INSERT);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Forcing update() ...";
    b->save(tpoy, PersistenceBroker::UPDATE);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying erase() ...";
    b->erase(tpoy);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying retrieve(ObjectId&) ...";
    b->retrieve<Y>(oid);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  try {
    cout << "Trying retrieve<Y>(q) ...";
    b->retrieve<Y>(q);
  }
  catch (PL::Exception& e) {
    cerr << endl << e << endl;
  }
  cout << "  ok" << endl;

  return 0;
}
