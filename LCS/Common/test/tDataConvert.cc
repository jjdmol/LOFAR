//# tDataConvert.cc: Test program for DataConvert functions
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


#include <Common/DataConvert.h>
#include <Common/Debug.h>

using namespace LOFAR;

int main()
{
  try {
    {
      // Swap 2 byte integer and check if correct.
      // Double swapping should give the original.
      uint16 v1 = 0xbecd;
      uint16 v2 = v1;
      Assert (byteSwap(byteSwap(v1)) == v2);
      byteSwap16 (&v2, &v1, 1);
      Assert (v2 == 0xcdbe);
    }
    {
      // Swap 4 byte integer and check if correct.
      // Double swapping should give the original.
      uint32 v1 = 0xbecd0256;
      uint32 v2 = v1;
      Assert (byteSwap(byteSwap(v1)) == v2);
      byteSwap32 (&v2, &v1, 1);
      Assert (v2 == 0x5602cdbe);
      byteSwap16 (&v2, &v1, 2);
      Assert (v2 == 0xcdbe5602);
    }
    {
      // Swap 8 byte integer and check if correct.
      // Double swapping should give the original.
      uint64 v1 = 0xbecd025613890574ULL;
      uint64 v2 = v1;
      Assert (byteSwap(byteSwap(v1)) == v2);
      byteSwap64 (&v2, &v1, 1);
      Assert (v2 == 0x740589135602cdbeULL);
      byteSwap32 (&v2, &v1, 2);
      Assert (v2 == 0x5602cdbe74058913ULL);
    }
    {
      // Swap 2 byte integers in a vector.
      // Double swapping should give the original.
      uint16 v1[3];
      v1[0]=0x0123; v1[1]=0x4567; v1[2]=0x89ab;
      uint16 v2[3];
      byteSwap16 (v2, v1, 3);
      Assert (v2[0] == 0x2301);
      Assert (v2[1] == 0x6745);
      Assert (v2[2] == 0xab89);
      byteSwap16 (v2, 3);
      Assert (v2[0] == v1[0]);
      Assert (v2[1] == v1[1]);
      Assert (v2[2] == v1[2]);
    }
    {
      // Swap 4 byte integers in a vector.
      // Double swapping should give the original.
      uint32 v1[3];
      v1[0]=0x01233210; v1[1]=0x45677654; v1[2]=0x89abba98;
      uint32 v2[3];
      byteSwap32 (v2, v1, 3);
      Assert (v2[0] == 0x10322301);
      Assert (v2[1] == 0x54766745);
      Assert (v2[2] == 0x98baab89);
      byteSwap32 (v2, 3);
      Assert (v2[0] == v1[0]);
      Assert (v2[1] == v1[1]);
      Assert (v2[2] == v1[2]);
    }
    {
      // Swap 8 byte integers in a vector.
      // Double swapping should give the original.
      uint64 v1[3];
      v1[0]=0x0123321045677654ULL; v1[1]=0x1234567887654321ULL;
      v1[2]=0x89abcdeffedcba98ULL;
      uint64 v2[3];
      byteSwap64 (v2, v1, 3);
      Assert (v2[0] == 0x5476674510322301ULL);
      Assert (v2[1] == 0x2143658778563412ULL);
      Assert (v2[2] == 0x98badcfeefcdab89ULL);
      byteSwap64 (v2, 3);
      Assert (v2[0] == v1[0]);
      Assert (v2[1] == v1[1]);
      Assert (v2[2] == v1[2]);
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}
