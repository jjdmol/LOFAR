//#  tAttrib.cc: program to test the makeAttrib method(s).
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <PL/Attrib.h>
#include "PO_tAttrib.h"

using namespace LOFAR::PL;
using namespace dtl;
using namespace std;

// class Z
// {
// public:
//   Z() : s("Z") {}
// private:
//   friend class TPersistentObject<Z>;
//   string s;
// };

// class Y
// {
// public:
//   Y() : s("Y") {}
// private:
//   friend class TPersistentObject<Y>;
//   string s;
//   Z z;
// };

// class X
// {
// public:
//   X() : s("X") {}
// private:
//   friend class TPersistentObject<X>;
//   string s;
// };

// class A
// {
// public:
//   A() : s("A") {}
// private:
//   friend class TPersistentObject<A>;
//   string s;
// };

// class B : public A
// {
// public:
//   B() : s("B") {}
// private:
//   friend class TPersistentObject<B>;
//   string s;
//   X x;
// };

// class C : public B
// {
// public:
//   C() : s("C") {}
// private:
//   friend class TPersistentObject<C>;
//   string s;
//   Y y;
// };


// namespace LOFAR { 

//   namespace PL {

//     template<> struct DBRep<Z> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<Z>::toDBRep(DBRep<Z>& dest) const {}
//     template<> void TPersistentObject<Z>::fromDBRep(const DBRep<Z>& src) {}
//     template<> void TPersistentObject<Z>::init()
//     { metaData().tableName() = "Z"; }
//     template<> void TPersistentObject<Z>::initAttribMap()
//     { theirAttribMap["s"] = "STR_Z"; }

//     template<> struct DBRep<Y> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<Y>::toDBRep(DBRep<Y>& dest) const {}
//     template<> void TPersistentObject<Y>::fromDBRep(const DBRep<Y>& src) {}
//     template<> void TPersistentObject<Y>::init()
//     {
//       metaData().tableName() = "Y"; 
// //       Pointer p(new TPersistentObject<Z>(data().z));
// //       p->metaData().ownerOid() = metaData().oid();
// //       ownedPOs().push_back(p);
//     }
//     template<> void TPersistentObject<Y>::initAttribMap()
//     { 
//       theirAttribMap["s"] = "STR_Y"; 
// //       theirAttribMap["z"] = 
// //         "@" + string(typeid(TPersistentObject<Z>).name()); 
//       theirAttribMap["z.s"] = "STR_Z";
//     }

//     template<> struct DBRep<X> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<X>::toDBRep(DBRep<X>& dest) const {}
//     template<> void TPersistentObject<X>::fromDBRep(const DBRep<X>& src) {}
//     template<> void TPersistentObject<X>::init()
//     { metaData().tableName() = "X"; }
//     template<> void TPersistentObject<X>::initAttribMap()
//     { theirAttribMap["s"] = "STR_X"; }

//     template<> struct DBRep<A> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<A>::toDBRep(DBRep<A>& dest) const {}
//     template<> void TPersistentObject<A>::fromDBRep(const DBRep<A>& src) {}
//     template<> void TPersistentObject<A>::init()
//     { metaData().tableName() = "A"; }
//     template<> void TPersistentObject<A>::initAttribMap()
//     { theirAttribMap["s"] = "STR_A"; }

//     template<> struct DBRep<B> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<B>::toDBRep(DBRep<B>& dest) const {}
//     template<> void TPersistentObject<B>::fromDBRep(const DBRep<B>& src) {}
//     template<> void TPersistentObject<B>::init()
//     { 
//       metaData().tableName() = "B"; 
//       {
//         Pointer p(new TPersistentObject<A>(data()));
//         p->metaData().ownerOid() = metaData().oid();
//         ownedPOs().push_back(p);
//       }
//       {
//         Pointer p(new TPersistentObject<X>(data().x));
//         p->metaData().ownerOid() = metaData().oid();
//         ownedPOs().push_back(p);
//       }
//     }
//     template<> void TPersistentObject<B>::initAttribMap() 
//     { 
//       theirAttribMap["s"] = "STR_B"; 
// //       theirAttribMap["A::"] = 
// //         "@" + string(typeid(TPersistentObject<A>).name()); 
//       theirAttribMap["A::s"] = "STR_A";
//       theirAttribMap["x"] = 
//         "@" + string(typeid(TPersistentObject<X>).name()); 
//     }

//     template<> struct DBRep<C> { void bindCols(dtl::BoundIOs&) {} };
//     template<> void TPersistentObject<C>::toDBRep(DBRep<C>& dest) const {}
//     template<> void TPersistentObject<C>::fromDBRep(const DBRep<C>& src) {}
//     template<> void TPersistentObject<C>::init()
//     {
//       Pointer p;
//       p.reset(new TPersistentObject<B>(data()));
//       p->metaData().ownerOid() = metaData().oid();
//       ownedPOs().push_back(p);
//       p.reset(new TPersistentObject<Y>(data().y));
//       p->metaData().ownerOid() = metaData().oid();
//       ownedPOs().push_back(p);
//       metaData().tableName() = "C"; 
//     }
//     template<> void TPersistentObject<C>::initAttribMap()
//     {
//       theirAttribMap["s"] = "STR_C";
//       theirAttribMap["B::"] =  
//         "@" + string(typeid(TPersistentObject<B>).name());
//       theirAttribMap["y"] = 
//         "@" + string(typeid(TPersistentObject<Y>).name());
//     }

//   }
// }
    

int main(int argc, const char* argv[])
{

  Debug::initLevels(argc, argv);

  try {

    cout << "attrib<Z>(\"s\")       = " << attrib<Z>("s")       << endl;
    cout << "attrib<Y>(\"s\")       = " << attrib<Y>("s")       << endl;
    cout << "attrib<Y>(\"z.s\")     = " << attrib<Y>("z.s")     << endl;
    cout << "attrib<X>(\"s\")       = " << attrib<X>("s")       << endl;

    cout << "attrib<A>(\"s\")       = " << attrib<A>("s")       << endl;
    cout << "attrib<B>(\"s\")       = " << attrib<B>("s")       << endl;
    cout << "attrib<B>(\"x.s\")     = " << attrib<B>("x.s")     << endl;
    cout << "attrib<B>(\"A::s\")    = " << attrib<B>("A::s")    << endl;

    cout << "attrib<C>(\"s\")       = " << attrib<C>("s")       << endl;
    cout << "attrib<C>(\"y.s\")     = " << attrib<C>("y.s")     << endl;
    cout << "attrib<C>(\"y.z.s\")   = " << attrib<C>("y.z.s")   << endl;
    cout << "attrib<C>(\"B::s\")    = " << attrib<C>("B::s")    << endl;
    cout << "attrib<C>(\"B::x.s\")  = " << attrib<C>("B::x.s")  << endl;
    cout << "attrib<C>(\"B::A::s\") = " << attrib<C>("B::A::s") << endl;

  }

  catch (LOFAR::Exception& e) {
    cerr << e << endl;
  }

  return 0;
}
