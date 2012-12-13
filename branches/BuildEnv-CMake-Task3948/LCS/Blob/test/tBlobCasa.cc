//# tBlobCasa.cc: Test program for Blob functions of casa arrays
//#
//# Copyright (C) 2007
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

#ifdef HAVE_AIPSPP

#include <Blob/BlobArray.h>
#include <Blob/BlobString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/Complex.h>

using namespace LOFAR;


void doOut (BlobOBuffer& bb)
{
  BlobOStream bs(bb);
  bs.putStart ("test", 1);
  // Create and fill a 3-dim array.
  casa::IPosition shp(3,2,3,4);
  bs << shp;
  casa::Array<double> arr(shp);
  indgen (arr);
  bs << arr;
  // Create an int vector.
  std::vector<casa::Int64> vec(2);
  vec[0] = 10;
  vec[1] = 11;
  bs << vec;
  // Create an empty vector.
  casa::Vector<casa::String> empVec;
  bs << empVec;
  // Create a complex vector.
  casa::Vector<casa::DComplex> vecc(1);
  vecc[0] = casa::DComplex(2,3);
  bs << vecc;
  // Create a string vector.
  casa::Vector<casa::String> vecs(2);
  vecs[0] = "str1";
  vecs[1] = "str1a";
  bs << vecs;
  // Write the shape and the arrays.
  //  bs << shp << arr << vec << empVec << vecc << vecs;
  bs.putEnd();
}

void doIn (BlobIBuffer& bb)
{
  BlobIStream bs(bb);
  bs.getStart ("test");
  // Read the shape as a vector.
  std::vector<casa::Int64> vec(1,100);
  bs >> vec;
  // Read the array.
  casa::Array<double> arr;
  bs >> arr;
  // Check the values.
  ASSERT (vec.size() == 3);
  ASSERT (vec[0] == 2  &&  vec[1] == 3  &&  vec[2] == 4);
  casa::IPosition shp(3,2,3,4);
  ASSERT (arr.shape() == shp);
  casa::Array<double> arr2(shp);
  indgen(arr2);
  ASSERT (allEQ(arr, arr2));
  // Read the vector as a shape.
  bs >> shp;
  ASSERT (shp.size() == 2);
  ASSERT (shp[0] = 10  && shp[1] == 11);
  // Get the empty vector.
  casa::Vector<casa::String> vecs(1);
  vecs[0] = "a";
  bs >> vecs;
  ASSERT (vecs.size() == 0);
  // Get the complex vector.
  casa::Vector<casa::DComplex> vecc;
  bs >> vecc;
  ASSERT (vecc.size() == 1);
  ASSERT (vecc[0] == casa::DComplex(2,3));
  // Get the string vector.
  bs >> vecs;
  ASSERT (vecs.size() == 2);
  ASSERT (vecs[0] == "str1");
  ASSERT (vecs[1] == "str1a");
  bs.getEnd();
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
