//#  CyclicCounterTest.cc: a test program for the CyclicCounter class
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include "CyclicCounter.h"
#include <stdio.h>

using namespace LOFAR;


int main (int argc, const char** argv)
{ 
  try {
    
    CyclicCounter cnt1(100);
    CyclicCounter cnt2(100);

    cout << "+cnt1 test" << endl;
    for (int i=0;i<150;i++) {
      cnt1+=10;
      cnt2-=2;
      cout << "count1 is " << cnt1 << " ,count2 is " << cnt2 << endl;
      if (cnt1 > 50) {
        cout << "count1 is groter dan 50"<< endl;
      }
      if (cnt2 < 50) {
        cout << "count2 is kleiner dan 50"<< endl;
      }
      if (cnt1 < cnt2) {
         cout << "count1 is kleiner dan count2"<< endl;
      }
      if (cnt1 > cnt2) {
         cout << "count1 is groter dan count2"<< endl;
      }
      cout << endl;
    } 
    

    

     
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "Test OK" << endl;
  return 0;
}

