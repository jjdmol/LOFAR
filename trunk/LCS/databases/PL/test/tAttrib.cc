#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <PL/Attrib.h>

using namespace LOFAR::PL;
using namespace dtl;
using namespace std;


/****************      User-defined classes      ****************\
|*                                                              *|
|*    Here are the user-defined classes. Note the addition of   *|
|*    the friendship declaration.                               *|
|*                                                              *|
\****************************************************************/

class X
{
public:
  X() : s("X") {}
private:
  friend class TPersistentObject<X>;
  string s;
};

class A
{
public:
  A() : s("A") {}
private:
  friend class TPersistentObject<A>;
  string s;
};

class B : public X
{
public:
  B() : s("B") {}
private:
  friend class TPersistentObject<B>;
  string s;
  A a;
};

class C : public A
{
public:
  C() : s("C") {}
private:
  friend class TPersistentObject<C>;
  string s;
};

class D : public B
{
public:
  D() : s("D") {}
private:
  friend class TPersistentObject<D>;
  string s;
  C c;
};


/******** PO adapter classes for the user-defined classes ********\
|*                                                               *|
|*    This code will normally be generated automatically using   *|
|*    the PL gentools.                                           *|
|*                                                               *|
\*****************************************************************/

namespace LOFAR { 

  namespace PL {

    template<> struct DBRep<X> { void bindCols(dtl::BoundIOs&) {} };
    template<> void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const {}
    template<> void TPersistentObject<X>::fromDBRep(const DBRep<X>& src) {}
    template<> void TPersistentObject<X>::init()
    { metaData().tableName() = "X"; }
    template<> void TPersistentObject<X>::initAttribMap()
    { theirAttribMap["s"] = "STR_X"; }

    template<> struct DBRep<A> { void bindCols(dtl::BoundIOs&) {} };
    template<> void TPersistentObject<A>::toDBRep(DBRep<A>& dest) const {}
    template<> void TPersistentObject<A>::fromDBRep(const DBRep<A>& src) {}
    template<> void TPersistentObject<A>::init()
    { metaData().tableName() = "A"; }
    template<> void TPersistentObject<A>::initAttribMap()
    { theirAttribMap["s"] = "STR_A"; }

    template<> struct DBRep<B> { void bindCols(dtl::BoundIOs&) {} };
    template<> void TPersistentObject<B>::toDBRep(DBRep<B>& dest) const {}
    template<> void TPersistentObject<B>::fromDBRep(const DBRep<B>& src) {}
    template<> void TPersistentObject<B>::init()
    { 
      Pointer p;
      p.reset(new TPersistentObject<X>(data()));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      p.reset(new TPersistentObject<A>(data().a));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      metaData().tableName() = "B"; 
    }
    template<> void TPersistentObject<B>::initAttribMap() 
    { 
      theirAttribMap["s"] = "STR_B"; 
      theirAttribMap["a"] = 
        "@" + string(typeid(TPersistentObject<A>).name()); 
      theirAttribMap["X::"]  = 
        "@" + string(typeid(TPersistentObject<X>).name());
    }

    template<> struct DBRep<C> { void bindCols(dtl::BoundIOs&) {} };
    template<> void TPersistentObject<C>::toDBRep(DBRep<C>& dest) const {}
    template<> void TPersistentObject<C>::fromDBRep(const DBRep<C>& src) {}
    template<> void TPersistentObject<C>::init()
    {
      Pointer p(new TPersistentObject<A>(data()));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      metaData().tableName() = "C"; 
    }
    template<> void TPersistentObject<C>::initAttribMap()
    {
      theirAttribMap["s"] = "STR_C";
      theirAttribMap["A::"] =  
        "@" + string(typeid(TPersistentObject<A>).name());
    }

    template<> struct DBRep<D> { void bindCols(dtl::BoundIOs&) {} };
    template<> void TPersistentObject<D>::toDBRep(DBRep<D>& dest) const {}
    template<> void TPersistentObject<D>::fromDBRep(const DBRep<D>& src) {}
    template<> void TPersistentObject<D>::init()
    {
      Pointer p;
      p.reset(new TPersistentObject<B>(data()));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      p.reset(new TPersistentObject<C>(data().c));
      p->metaData().ownerOid() = metaData().oid();
      ownedPOs().push_back(p);
      metaData().tableName() = "D"; 
    }
    template<> void TPersistentObject<D>::initAttribMap()
    {
      theirAttribMap["s"] = "STR_D";
      theirAttribMap["c"] = 
        "@" + string(typeid(TPersistentObject<C>).name());
      theirAttribMap["B::"] =  
        "@" + string(typeid(TPersistentObject<B>).name());
    }

  }
}
    

/****************      The main test program     ****************\
\****************************************************************/

int main(int argc, const char* argv[])
{

  Debug::initLevels(argc, argv);

  try {

    TPersistentObject<A> a;
    TPersistentObject<B> b;
    TPersistentObject<C> c;
    TPersistentObject<D> d;

    cout << "attrib<A>(\"s\")       = " << attrib<A>("s")       << endl;
    cout << "attrib<B>(\"a.s\")     = " << attrib<B>("a.s")     << endl;
//     cout << "attrib<C>(\"A::s\")    = " << attrib<C>("A::s")    << endl;
//     cout << "attrib<D>(\"B::a.s\"   = " << attrib<D>("B::a.s")  << endl;
//     cout << "attrib<D>(\"c.A::s\")  = " << attrib<D>("c.A::s")  << endl;
    cout << "attrib<B>(\"s\")       = " << attrib<B>("s")       << endl;
//     cout << "attrib<D>(\"B::s\")    = " << attrib<D>("B::s")    << endl;
    cout << "attrib<C>(\"s\")       = " << attrib<C>("s")       << endl;
    cout << "attrib<D>(\"c.s\")     = " << attrib<D>("c.s")     << endl;
    cout << "attrib<D>(\"s\")       = " << attrib<D>("s")       << endl;
//     cout << "attrib<B>(\"X::s\")    = " << attrib<B>("X::s")    << endl;
//     cout << "attrib<D>(\"B::X::s\") = " << attrib<D>("B::X::s") << endl;
//     cout << "attrib<D>(\"X::s\")    = " << attrib<D>("X::s")    << endl;
    
  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
  }

  return 0;
}
