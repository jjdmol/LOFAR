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
#include <Common/Stopwatch.h>

int main()
{
  try {
    {
      const int n = 10000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = n-i-1;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath 10^4 " << timer.sdelta() << endl;
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
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath order" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	Assert (vec(i) == double(i));
      }
    }
    {
      const int n = 10000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = 0.;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath equal" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	Assert (vec(i) == double(0));
      }
    }
    {
      const int n = 100000;
      LoVec_double vec(n);
      for (int i=0; i<n; i++) {
	vec(i) = n-i-1;
      }
      Stopwatch timer;
      for (int j=0; j<100; j++) {
	LCSMath::sort (vec);
      }
      cout << "100x LCSMath 10^5 " << timer.sdelta() << endl;
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
      Stopwatch timer;
      for (int j=0; j<10; j++) {
	LCSMath::sort (vec);
      }
      cout << " 10x LCSMath 10^6 " << timer.sdelta() << endl;
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
      Stopwatch timer;
      for (int j=0; j<10; j++) {
	LCSMath::sort (vec);
      }
      cout << " 10x LCSMath order" << timer.sdelta() << endl;
      for (int i=0; i<n; i++) {
	Assert (vec(i) == double(i));
      }
    }
  } catch (...) {
    cout << "Unexpected exception" << endl;
    return 1;
  }
  return 0;
}
