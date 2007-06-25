//# tBlobCasa.cc: Test program for Blob functions of casa arrays
//#
//# Copyright (C) 2007
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#ifdef HAVE_AIPSPP

#include <Blob/BlobArray.h>
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

using namespace LOFAR;


void doOut (BlobOBuffer& bb)
{
  BlobOStream bs(bb);
  bs.putStart ("test", 1);
  // Create and fill a 3-dim array.
  casa::IPosition shp(3,2,3,4);
  casa::Array<double> arr(shp);
  indgen (arr);
  // Write the shape and the array.
  bs << shp << arr;
  bs.putEnd();
}

void doIn (BlobIBuffer& bb)
{
  BlobIStream bs(bb);
  bs.getStart ("test");
  // Read the shape as a vector.
  std::vector<casa::Int> vec;
  bs >> vec;
  // Read the array.
  casa::Array<double> arr;
  bs >> arr;
  bs.getEnd();
  // Check the values.
  ASSERT (vec.size() == 3);
  ASSERT (vec[0] == 2  &&  vec[1] == 3  &&  vec[2] == 4);
  casa::IPosition shp(3,2,3,4);
  ASSERT (arr.shape() == shp);
  casa::Array<double> arr2(shp);
  indgen(arr2);
  ASSERT (allEQ(arr, arr2));
}

int main()
{
  try {
    INIT_LOGGER("tBlobCasa");
    {
      // Create the blob in memory.
      BlobString str;
      BlobOBufString bob(str);
      doOut (bob);
      BlobIBufString bib(str);
      doIn (bib);
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

#else
int main()
{
  return 3;   // skipped
}
#endif
