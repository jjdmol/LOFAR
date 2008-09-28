//#  tBacktrace.cc: one line description
//#
//#  Copyright (C) 2002-2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#ifdef HAVE_BACKTRACE
# include <Common/Backtrace.h>
#endif
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>

using namespace LOFAR;

struct J
{ 
  void doIt() 
  { 
    ASSERTSTR(false, "Ouch!"); 
  }
};

struct I 
{
  void doIt() { j.doIt(); }
  J j;
};

struct H
{
  void doIt() { i.doIt(); }
  I i;
};

struct G
{
  void doIt() { h.doIt(); }
  H h;
};

struct F 
{
  void doIt() { g.doIt(); }
  G g;
};

struct E 
{
  void doIt() { f.doIt(); }
  F f; 
};

struct D
{
  void doIt() { e.doIt(); }
  E e; 
};

struct C 
{
  void doIt() { d.doIt(); }
  D d; 
};

struct B 
{
  void doIt() { c.doIt(); }
  C c;
};

struct A 
{
  void doIt() { b.doIt(); }
  B b; 
};

int main()
{
  INIT_LOGGER("tBacktrace");
  try {
    A().doIt();
  } catch (Exception& e) {
    std::cerr << e << std::endl;
  }
  return 0;
}
