#include <Common/fwd/PL.h>
#include <PL/TPersistentObject.h>
#include <PL/PersistenceBroker.h>
#include <PL/DBRep.h>
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

    template<> struct DBRep<Bar>
    {
      void bindCols(dtl::BoundIOs& cols) {}
    };

    template<>
    void TPersistentObject<Bar>::toDBRep(DBRep<Bar>& dest) const
    {
    }

    template<>
    void TPersistentObject<Bar>::fromDBRep(const DBRep<Bar>& src)
    {
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


    template<> struct DBRep<Foo> 
    {
      void bindCols(dtl::BoundIOs& cols) {}
    };

    template<>
    void TPersistentObject<Foo>::toDBRep(DBRep<Foo>& dest) const
    {
    }

    template<>
    void TPersistentObject<Foo>::fromDBRep(const DBRep<Foo>& src)
    {
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

