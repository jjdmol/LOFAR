//#  tDebug.cc: Test program for Debug class
//#
//#  Copyright (C) 2002
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

#include <Common/Debug.h>

using namespace LOFAR;

void a()
{
  TRACERPF4 ("msg1");
  TRACERPFN4 (trac1,"msg2");
  TRACER4("intermediate");
  cout << "a debug: " << getDebugContext().name() << endl;
}




class XX
{
public:
  XX() {cout << "XX debug: " << getDebugContext().name() << endl;}
  LocalDebugContext;
};
InitDebugContext(XX,"XXD");

class ZZ
{
public:
  ZZ() {cout << "ZZ debug: " << getDebugContext().name() << endl;}
  LocalDebugContext;
};
InitDebugContext(ZZ,"ZZD");

class YY
{
public:
  YY() {cout << "YY debug: " << getDebugContext().name() << endl;}
  ImportDebugContext(XX);
};

#ifdef getDebugContext
#undef getDebugContext
#endif

void f()
{
  YY y1;
  XX x;
  ZZ z;
  YY y2;
  cout << "f debug: " << getDebugContext().name() << endl;
 
  // Some dirty code to prevent "unused variable" warnings during compilation
  void* ptr;
  ptr = (void*)&y1;
  ptr = (void*)&x;
  ptr = (void*)&z;
  ptr = (void*)&y2;
}


int main()
{
  // Use highest trace level.
  getDebugContext().setLevel(3);
  a();
  f();

  int i0=0;
  int i1=1;
  try {
    AssertMsg (i0==i1, "should give exception");
  } catch (LOFAR::Exception& x) {
    cout << "caught LOFAR::Exception" <<endl;
    cout << x.what() << endl;
  } catch (std::exception& x) {
    cout << "caught unexpected std::exception" <<endl;
    cout << x.what() << endl;
    return 1;
  } catch (...) {
    cout << "caught unknown exception" <<endl;
    return 1;
  }  
  return 0;
}
