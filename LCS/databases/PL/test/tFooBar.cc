#include <Common/fwd/PL.h>
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/Attrib.h>
#include <iostream>
#include <typeinfo>

using namespace std;
using namespace dtl;

class Bar
{
public:
  Bar(const string& s="") : text(s) {}
  friend ostream& operator<<(ostream& os, const Bar& bar) {
    os << bar.text << endl;
    return os;
  }
private:
  friend class TPersistentObject<Bar>;
  string text;
};

class Foo
{
public:
  Foo() : val(0) {}
  Foo(int i, const string& s) : val(i), bar(s) {}
  friend ostream& operator<<(ostream& os, const Foo& foo) {
    os << foo.val << foo.bar << endl;
    return os;
  }
private:
  friend class TPersistentObject<Foo>;
  int val;
  Bar bar;
};

namespace LOFAR { 

  namespace PL {

    template<> struct DBRep<Bar> : public DBRepMeta
    {
//       ObjectId::oid_t  itsOid;
//       ObjectId::oid_t  itsOwnerOid;
    };

    template<>
    void BCA<Bar>::operator()(BoundIOs& cols, DataObj& rowbuf) 
    {
// 	  cols["ObjID"]  == rowbuf.itsOid;
// 	  cols["Owner"]  == rowbuf.itsOwnerOid;
    }
    
    template<>
    void TPersistentObject<Bar>::fromDBRep(const DBRep<Bar>& org)
    {
//       metaData().oid()->set(org.itsOid);
//       metaData().ownerOid()->set(org.itsOwnerOid);
    }

    template<>
    void TPersistentObject<Bar>::toDBRep(DBRep<Bar>& dest) const
    {
//       dest.itsOid   = metaData().oid()->get();
//       dest.itsOwnerOid = metaData().ownerOid()->get();
    }

    template<>
    void TPersistentObject<Bar>::initAttribMap()
    {
      theirAttribMap["text"] = "TEXT";
    }

    template<>
    void TPersistentObject<Bar>::init()
    {
      tableName("BAR");
    }

    
    template<> struct DBRep<Foo> : public DBRepMeta
    {
//       ObjectId::oid_t  itsOid;
//       ObjectId::oid_t  itsOwnerOid;
    };

    template<>
    void BCA<Foo>::operator()(BoundIOs& cols, DataObj& rowbuf) 
    {
// 	  cols["ObjID"]  == rowbuf.itsOid;
// 	  cols["Owner"]  == rowbuf.itsOwnerOid;
    }
    
    template<>
    void TPersistentObject<Foo>::fromDBRep(const DBRep<Foo>& org)
    {
//       metaData().oid()->set(org.itsOid);
//       metaData().ownerOid()->set(org.itsOwnerOid);
    }

    template<>
    void TPersistentObject<Foo>::toDBRep(DBRep<Foo>& dest) const
    {
//       dest.itsOid   = metaData().oid()->get();
//       dest.itsOwnerOid = metaData().ownerOid()->get();
    }

    template<>
    void TPersistentObject<Foo>::initAttribMap()
    {
      theirAttribMap["val"] = "VAL";
      theirAttribMap["bar"] = 
        "@" + string(typeid(TPersistentObject<Bar>).name());
    }

    template<>
    void TPersistentObject<Foo>::init()
    {
      Pointer p(new TPersistentObject<Bar>(itsObjectPtr->bar));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      tableName("FOO");
    }

  }

}

using namespace LOFAR::PL;
using namespace LOFAR;

int main()
{
  Foo foo(1,"Hello");
  Bar bar("World");

  TPersistentObject<Foo> tpoFoo(foo);
  TPersistentObject<Bar> tpoBar(bar);

//   cout << tpoFoo.data() << endl;
//   cout << tpoBar.data() << endl;

  try {
    cout << "attrib<Foo>(\"bar.text\") = " 
         << attrib<Foo>("bar.text") << endl;
  }
  catch (Exception& e) {
    cerr << endl << e << endl;
  }

  return 0;
}

