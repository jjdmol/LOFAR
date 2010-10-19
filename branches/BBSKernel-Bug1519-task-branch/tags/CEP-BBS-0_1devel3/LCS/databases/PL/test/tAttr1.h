//#  tAttr1.h: program to test the makeAttrib method(s).
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

#ifndef PL_TATTR1_H
#define PL_TATTR1_H

#include <PL/PLfwd.h>

class A {
public:
  friend class LOFAR::PL::TPersistentObject<A>;
  int& s() {return itsS;}
private:
  int itsS;
};
class B : public A {
public:
  friend class LOFAR::PL::TPersistentObject<B>;
  int& bs() {return itsS;}
  int& bas() {return A::s();}
private:
  int itsS;
};
class C : public A {
public:
  friend class LOFAR::PL::TPersistentObject<C>;
  int& cs() {return itsS;}
  int& bs() {return itsB.bs();}
  int& bas() {return itsB.bas();}
  int& cas() {return A::s();}
private:
  int itsS;
  B   itsB;
};
class D {
public:
  friend class LOFAR::PL::TPersistentObject<D>;
  int& ds() {return itsS;}
private:
  int itsS;
};
class E {
public:
  friend class LOFAR::PL::TPersistentObject<E>;
  int& es() {return itsS;}
  int& ds() {return itsD.ds();}
private:
  int itsS;
  D   itsD;
};
class F : public C {
public:
  friend class LOFAR::PL::TPersistentObject<F>;
  int& bs() {return C::bs();}
  int& cs() {return C::cs();}
  int& bas() {return C::bas();}
  int& cas() {return C::cas();}
  int& ds() {return itsE.ds();}
  int& es() {return itsE.es();}
  int& fs() {return itsS;}
private:
  int itsS;
  E   itsE;
};

#endif
