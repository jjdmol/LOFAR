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

    template<> class DBRep<X> : public DBRepMeta
    {
    public:
      void bindCols(dtl::BoundIOs& cols) {}
      void toDBRep(const X& src) {}
      void fromDBRep(X& dest) const {}
    };

    template<> void TPersistentObject<X>::initAttribMap()
    {
    }

    template<> void TPersistentObject<X>::init() 
    { 
      metaData().tableName() = "X"; 
    }
    
    template<> class DBRep<Y> : public DBRepMeta
    {
    public:
      void bindCols(dtl::BoundIOs& cols) {}
      void toDBRep(const Y& src) {}
      void fromDBRep(Y& dest) const {}
    };

    template<> void TPersistentObject<Y>::initAttribMap()
    {
      theirAttribMap["hello"] = "@Hello@";
    }

    template<> void TPersistentObject<Y>::init() 
    {
      Pointer p(new TPersistentObject<X>(*itsObjectPtr));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      tableName("Y");
      initAttribMap();
    }

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
    cout << "attrib<Y>(\"hello.world\") = " 
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
