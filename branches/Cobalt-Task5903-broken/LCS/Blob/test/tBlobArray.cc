//# tBlobArray.cc: Test program for BlobArray functions
//#
//# Copyright (C) 2003
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

#include <Blob/BlobArray.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufNull.h>
#include <iostream>
#include <fstream>

using namespace LOFAR;

uint doOut (BlobOBuffer& bb)
{
  // Write a vector into the blob.
  BlobOStream bs(bb);
  std::vector<double> vec(3);
  vec[0] = double(2);
  vec[1] = double(1e10);
  vec[2] = double(-3.1);
  bs << vec;
  // Check if alignment is done properly.
  int pos = bs.tellPos();
  ASSERT (bs.align(0) == 0);
  ASSERT (bs.align(1) == 0);
  ASSERT (bs.align(128) == uint(128-pos));
  // Reserve space for a complex array and the offset of the array.
  return setSpaceBlobArray1<fcomplex> (bs, true, 2);
}

void doIn (BlobIBuffer& bb, bool read2=false)
{
  // Read the vector from the blob and check if it is correct.
  BlobIStream bs(bb);
  std::vector<double> vec(5);
  bs >> vec;
  ASSERT (vec.size() == 3);
  ASSERT (vec[0] == double(2));
  ASSERT (vec[1] == double(1e10));
  ASSERT (vec[2] == double(-3.1));
  if (read2) {
    // Do alignment and check if done as expected.
    int pos = bs.tellPos();
    ASSERT (bs.align(0) == 0);
    ASSERT (bs.align(1) == 0);
    ASSERT (bs.align(128) == uint(128-pos));
    // Read and check the complex vector.
    std::vector<fcomplex> vecc;
    bs >> vecc;
    ASSERT (vecc.size() == 2);
    ASSERT (vecc[0] == makefcomplex(2,3));
    ASSERT (vecc[1] == makefcomplex(-1,1e10));
  }
}

int main()
{
  try {
    INIT_LOGGER("tBlobArray");
    {
      {
	// Create the blob in a file.
	std::ofstream os ("tBlobArray_tmp.dat");
	BlobOBufStream bob(os);
        doOut (bob);
      }
      {
	// Read it back from the file.
	std::ifstream is ("tBlobArray_tmp.dat");
	BlobIBufStream bib(is);
	doIn (bib);
      }
    }
    {
      // Create the blob in memory.
      BlobString str(BlobStringType(false));
      BlobOBufString bob(str);
      uint cpos = doOut (bob);
      // Print the offset of the complex vector.
      std::cout << "stringpos=" << cpos << std::endl;
      // Get a pointer to the complex vector in the buffer and fill the vector.
      fcomplex* ptr = bob.getPointer<fcomplex> (cpos);
      ptr[0] = makefcomplex(2,3);
      ptr[1] = makefcomplex(-1,1e10);
      // Read the blob back from the string and check it.
      BlobIBufString bib(str);
      doIn (bib, true);
      {
	// Read back the blob another time, so create a new BlobIBuf object.
	BlobIBufString bob2(str);
	BlobIStream bs(bob2);
	std::vector<double> vec;
	bs >> vec;
	int pos = bs.tellPos();
	// Do the alignment again.
	ASSERT (bs.align(0) == 0);
	ASSERT (bs.align(1) == 0);
	ASSERT (bs.align(128) == uint(128-pos));
	// Interpret the complex vector directly as an array.
	// Check if the shape and values are correct.
	std::vector<uint64> shape;
	uint cpos = getSpaceBlobArray<fcomplex> (bs, true, shape, false);
	ASSERT (shape.size() == 1);
	ASSERT (shape[0] == 2);
	const fcomplex* ptr = bob2.getPointer<fcomplex> (cpos);
	ASSERT (ptr[0] == makefcomplex(2,3));
	ASSERT (ptr[1] == makefcomplex(-1,1e10));
      }
    }
    {
      // Create the blob in a null buffer to determine its length.
      BlobOBufNull bobn;
      uint cposn = doOut (bobn);
      std::cout << "stringpos=" << cposn << ' ' << bobn.size() << std::endl;
      // Create a non-expandable buffer of that size.
      // Create and interpret the blob again.
      BlobString str(BlobStringType(false), bobn.size(), false);
      BlobOBufString bob(str);
      uint cpos = doOut (bob);
      std::cout << "stringpos=" << cpos << std::endl;
      fcomplex* ptr = bob.getPointer<fcomplex> (cpos);
      ptr[0] = makefcomplex(2,3);
      ptr[1] = makefcomplex(-1,1e10);
      BlobIBufString bib(str);
      doIn (bib, true);
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

