//#  tAttrib.h: Class definitions for the test program tAttrib.cc
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

#ifndef LOFAR_PL_TEST_TATTRIB_H
#define LOFAR_PL_TEST_TATTRIB_H

#include <PL/PLfwd.h>

/****************      User-defined classes      ****************\
|*                                                              *|
|*    Here are the user-defined classes. Note the addition of   *|
|*    the friendship declaration.                               *|
|*                                                              *|
\****************************************************************/

class Y;

class Z
{
public:
  Z() : s("Z") {}
//   void set(const string& arg) { s = arg; }
//   const string get() const { return s; }
//   string& s_ref() { return s; }
//   const string& s_ref() const { return s; }
private:
  friend class LOFAR::PL::TPersistentObject<Z>;
//   friend class LOFAR::PL::TPersistentObject<Y>;
  string s;
};

class Y
{
public:
  Y() : s("Y") {}
private:
  friend class LOFAR::PL::TPersistentObject<Y>;
  string s;
  Z z;
};

class X
{
public:
  X() : s("X") {}
private:
  friend class LOFAR::PL::TPersistentObject<X>;
  string s;
};

class A
{
public:
  A() : s("A") {}
private:
  friend class LOFAR::PL::TPersistentObject<A>;
  string s;
};

class B : public A
{
public:
  B() : s("B") {}
private:
  friend class LOFAR::PL::TPersistentObject<B>;
  string s;
  X x;
};

class C : public B
{
public:
  C() : s("C") {}
private:
  friend class LOFAR::PL::TPersistentObject<C>;
  string s;
  Y y;
};

#endif
