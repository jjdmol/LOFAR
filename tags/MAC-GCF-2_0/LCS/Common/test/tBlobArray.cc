//# tBlobArray.cc: Test program for BlobArray functions
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

#include <Common/BlobArray.h>
#include <Common/BlobOBufStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufNull.h>
#include <Common/Debug.h>
#include <iostream>
#include <fstream>

using namespace LOFAR;

uint doOut (BlobOBuffer* bb)
{
  BlobOStream bs(bb, true);
  std::vector<double> vec(3);
  vec[0] = double(2);
  vec[1] = double(1e10);
  vec[2] = double(-3.1);
  bs << vec;
  int pos = bs.tellPos();
  Assert (bs.align(0) == 0);
  Assert (bs.align(1) == 0);
  Assert (bs.align(8) == 0);
  Assert (bs.align(128) == uint(128-pos));
  return setSpaceBlobArray1<fcomplex> (bs, 2, false);
}

void doIn (BlobIBuffer* bb, bool read2=false)
{
  BlobIStream bs(bb);
  std::vector<double> vec(5);
  bs >> vec;
  Assert (vec.size() == 3);
  Assert (vec[0] == double(2));
  Assert (vec[1] == double(1e10));
  Assert (vec[2] == double(-3.1));
  if (read2) {
    int pos = bs.tellPos();
    Assert (bs.align(0) == 0);
    Assert (bs.align(1) == 0);
    Assert (bs.align(8) == 0);
    Assert (bs.align(128) == uint(128-pos));
    std::vector<fcomplex> vecc;
    bs >> vecc;
    Assert (vecc.size() == 2);
    Assert (vecc[0] == fcomplex(2,3));
    Assert (vecc[1] == fcomplex(-1,1e10));
  }
}

int main()
{
  try {
    {
      {
	std::ofstream os ("tBlobArray_tmp.dat");
	BlobOBufStream bob(os);
        doOut (&bob);
      }
      {
	std::ifstream is ("tBlobArray_tmp.dat");
	BlobIBufStream bib(is);
	doIn (&bib);
      }
    }
    {
      BlobString str(BlobStringType(false));
      BlobOBufString bob(str);
      uint cpos = doOut (&bob);
      std::cout << "stringpos=" << cpos << std::endl;
      fcomplex* ptr = bob.getPointer<fcomplex> (cpos);
      ptr[0] = fcomplex(2,3);
      ptr[1] = fcomplex(-1,1e10);
      BlobIBufString bib(str);
      doIn (&bib, true);
      {
	BlobIBufString bob2(str);
	BlobIStream bs(&bob2);
	std::vector<double> vec;
	bs >> vec;
	int pos = bs.tellPos();
	Assert (bs.align(0) == 0);
	Assert (bs.align(1) == 0);
	Assert (bs.align(8) == 0);
	Assert (bs.align(128) == uint(128-pos));
	std::vector<uint32> shape;
	uint cpos = getSpaceBlobArray<fcomplex> (bs, shape, false);
	Assert (shape.size() == 1);
	Assert (shape[0] == 2);
	const fcomplex* ptr = bob2.getPointer<fcomplex> (cpos);
	Assert (ptr[0] == fcomplex(2,3));
	Assert (ptr[1] == fcomplex(-1,1e10));
      }
    }
    {
      BlobOBufNull bobn;
      uint cposn = doOut (&bobn);
      std::cout << "stringpos=" << cposn << ' ' << bobn.size() << std::endl;
      BlobString str(BlobStringType(false), bobn.size(), false);
      BlobOBufString bob(str);
      uint cpos = doOut (&bob);
      std::cout << "stringpos=" << cpos << std::endl;
      fcomplex* ptr = bob.getPointer<fcomplex> (cpos);
      ptr[0] = fcomplex(2,3);
      ptr[1] = fcomplex(-1,1e10);
      BlobIBufString bib(str);
      doIn (&bib, true);
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

