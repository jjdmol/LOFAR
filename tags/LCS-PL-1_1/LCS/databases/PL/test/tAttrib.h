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
#include <Common/lofar_string.h>

namespace LOFAR 
{

  //@defgroup tAttrib tAttrib: User-defined classes
  //@{

  class A
  {
  public:
    A() : s("A") {}
  private:
    friend class LOFAR::PL::TPersistentObject<A>;
    string s;
  };

  class B
  {
  public:
    B() : s("B") {}
  private:
    friend class LOFAR::PL::TPersistentObject<B>;
    string s;
  };

  class C : public A
  {
  public:
    C() : s("C") {}
  private:
    friend class LOFAR::PL::TPersistentObject<C>;
    string s;
  };

  class D : public B
  {
  public:
    D() : s("D") {}
  private:
    friend class LOFAR::PL::TPersistentObject<D>;
    string s;
    C c;
  };

  class E
  {
  public:
    E() : s("E") {}
  private:
    friend class LOFAR::PL::TPersistentObject<E>;
    string s;
  };

  class F
  {
  public:
    F() : s("F") {}
  private:
    friend class LOFAR::PL::TPersistentObject<F>;
    string s;
    E e;
  };

  class G : public D
  {
  public:
    G() : s("G") {}
  private:
    friend class LOFAR::PL::TPersistentObject<G>;
    string s;
    F f;
  };

  //@}

} // namespace LOFAR

#endif
