//# tBacktrace.cc: Unit test program for the Backtrace/Exception classes.
//#
//# Copyright (C) 2002-2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Exceptions.h>
#include <stdexcept>
#include <iostream>
#include <typeinfo>
#include <cstdlib>

using namespace LOFAR;
using namespace std;

Exception::TerminateHandler t(Exception::terminate);

struct J
{ 
  ~J()
  {
  }

  void doIt() 
  { 
//     terminate();
//     throw;
//     throw 1;
    throw runtime_error("Oops!");
    THROW (AssertError, "Ouch!"); 
    LOFAR::Exception* excp;
    char* p;
    cerr << "sizeof(AssertError) = " << sizeof(AssertError) << endl;
    try {
      ostringstream oss;
      oss << "cat /proc/" << getpid() << "/status";
      while (true) {
        system(oss.str().c_str());
        p = new char[2000000];
      }
    } catch (std::bad_alloc&) {
//       cerr << "Out of memory" << endl;
//       throw std::runtime_error("Out of memory");
//       throw AssertError("Out of memory", THROW_ARGS);
      THROW (AssertError, "Out of memory");
    }
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
  //   set_terminate(myterminate);
  //   try {
  A().doIt();
  //   } catch (Exception& e) {
  //     cerr << e << endl;
  //   }
  return 0;
}
