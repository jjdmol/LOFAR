//  tLCSMath.h: Test program for LCSMath
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <Math/LCSMath.h>
#include <Common/Lorrays-Blitz.h>
#include <Common/Debug.h>
#include <aips/Utilities/GenSort.h>
#include <aips/OS/Timer.h>

int main()
{
  {
    const int n = 10000;
    LoVec_double vec(n);
    for (int i=0; i<n; i++) {
      vec(i) = n-i-1;
    }
    Timer timer;
    for (int j=0; j<100; j++) {
      LCSMath::sort (vec);
    }
    timer.show ("100x LCSMath 10^4 ");
    for (int i=0; i<n; i++) {
      Assert (vec(i) == double(i));
    }
  }
  {
    const int n = 10000;
    LoVec_double vec(n);
    for (int i=0; i<n; i++) {
      vec(i) = i;
    }
    Timer timer;
    for (int j=0; j<100; j++) {
      LCSMath::sort (vec);
    }
    timer.show ("100x LCSMath order");
    for (int i=0; i<n; i++) {
      Assert (vec(i) == double(i));
    }
  }
  {
    const int n = 100000;
    LoVec_double vec(n);
    for (int i=0; i<n; i++) {
      vec(i) = n-i-1;
    }
    Timer timer;
    for (int j=0; j<100; j++) {
      LCSMath::sort (vec);
    }
    timer.show ("100x LCSMath 10^5 ");
    for (int i=0; i<n; i++) {
      Assert (vec(i) == double(i));
    }
  }
  {
    const int n = 1000000;
    LoVec_double vec(n);
    for (int i=0; i<n; i++) {
      vec(i) = n-i-1;
    }
    Timer timer;
    for (int j=0; j<10; j++) {
      LCSMath::sort (vec);
    }
    timer.show (" 10x LCSMath 10^6 ");
    for (int i=0; i<n; i++) {
      Assert (vec(i) == double(i));
    }
  }
  {
    const int n = 1000000;
    LoVec_double vec(n);
    for (int i=0; i<n; i++) {
      vec(i) = i;
    }
    Timer timer;
    for (int j=0; j<10; j++) {
      LCSMath::sort (vec);
    }
    timer.show (" 10x LCSMath order");
    for (int i=0; i<n; i++) {
      Assert (vec(i) == double(i));
    }
  }
}
