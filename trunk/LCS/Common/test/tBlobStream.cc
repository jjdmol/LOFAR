//# tBlobStream.cc: Test program for classes BlobOStream and BlobIStream
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

#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOBufStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobIBufVector.h>
#include <Common/BlobOBufVector.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobOBufNull.h>
#include <Common/Debug.h>
#include <iostream>
#include <fstream>
#include <vector>

int doOut (BlobOBuffer* bb, bool header8=false)
{
  uint len = 0;
  BlobOStream bos(bb, header8);
  Assert (bos.putStart ("test", 1) == 1);
  bos << true;
  bos << "abc";
  bos << int32(4);
  bos.put ("defg", 4);
  uint ln = 34;
  if (header8) ln = 40;
  Assert (bos.putEnd() == ln);
  len += ln;

  Assert (bos.putStart ("test1", 1) == 1);
  bos << int16(2);
  bos << fcomplex(1,2);
  Assert (bos.putStart ("test1a", 3) == 2);
  bos << std::string("defg");
  ln = 28;
  if (header8) ln = 32;
  Assert (bos.putEnd() == ln);
  ln += 29;
  if (header8) ln += 5;
  Assert (bos.putEnd() == ln);
  len += ln;

  Assert (bos.putStart ("test2", -1) == 1);
  bos << int64(100);
  bos << dcomplex(5,6);
  ln = 43;
  if (header8) ln = 48;
  Assert (bos.putEnd() == ln);
  len += ln;
  return len;
}

void doIn (BlobIBuffer* bb, bool header8=false)
{
  bool  valb;
  int16 val16;
  int64 val64;
  fcomplex     valfc;
  dcomplex     valdc;
  std::string  vals;

  BlobIStream bis(bb);
  Assert (bis.getStart ("test") == 1);
  bis >> valb;
  Assert (valb == true);
  bis >> vals;
  Assert (vals.size() == 3  &&  vals == "abc");
  bis >> vals;
  Assert (vals.size() == 4  &&  vals == "defg");
  uint ln = 34;
  if (header8) ln = 40;
  Assert (bis.getEnd() == ln);

  Assert (bis.getStart ("test1") == 1);
  bis >> val16;
  Assert (val16 == 2);
  bis >> valfc;
  Assert (valfc == fcomplex(1,2));
  Assert (bis.getStart ("test1a") == 3);
  bis >> vals;
  Assert (vals.size() == 4  &&  vals == "defg");
  ln = 28;
  if (header8) ln = 32;
  Assert (bis.getEnd() == ln);
  ln += 29;
  if (header8) ln += 5;
  Assert (bis.getEnd() == ln);

  Assert (bis.getStart ("test2") == -1);
  bis >> val64;
  Assert (val64 == 100);
  bis >> valdc;
  Assert (valdc == dcomplex(5,6));
  ln = 43;
  if (header8) ln = 48;
  Assert (bis.getEnd() == ln);
}


int main()
{
  try {
    {
      int len;
      {
	// Use a file output stream.
	std::ofstream os ("tBlobStream_tmp.dat");
	BlobOBufStream bob(os);
        len = doOut (&bob);
	Assert (int(os.tellp()) == len);
      }
      {
	// Use the file as input.
	std::ifstream is ("tBlobStream_tmp.dat");
	BlobIBufStream bib(is);
	doIn (&bib);
	Assert (int(is.tellg()) == len);
      }
    }
    {
      // Use a string output stream.
      std::ostringstream os;
      BlobOBufStream bob(os);
      doOut (&bob);
      // Reuse as input.
      std::istringstream is(os.str());
      BlobIBufStream bib(is);
      doIn (&bib);
      // Reuse as a char input.
      std::string str(os.str());
      BlobIBufChar bibc(str.data(), str.size());
      doIn (&bibc);
      // Copy to vector and use that.
      std::vector<uchar> vec(os.str().size());
      BlobIBufVector<uchar> bibv(vec);
      memcpy (&vec[0], os.str().data(), os.str().size());
      doIn (&bibv);
    }
    {
      // Use a null buffer.
      BlobOBufNull bob;
      doOut (&bob);
    }
    {
      // Use an expandable char buffer.
      BlobOBufChar bob(10,5);
      doOut (&bob);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (&bib);
    }
    {
      // Use an expandable char buffer; make header multiple of 8.
      BlobOBufChar bob(10,5);
      doOut (&bob, true);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (&bib, true);
    }
    {
      // Use an expandable unallocated char buffer.
      BlobOBufChar bob(0,0,10);
      doOut (&bob);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (&bib);
    }
    {
      // Use an unexpandable preallocated char buffer.
      uchar buf[200];
      BlobOBufChar bob(buf,200);
      doOut (&bob);
      Assert (bob.getBuffer() == buf);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (&bib);
    }
    {
      // Use an expandable preallocated char buffer.
      uchar buf[20];
      BlobOBufChar bob(buf,20,20);
      doOut (&bob);
      Assert (bob.getBuffer() != buf);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (&bib);
    }
    {
      // Use an expandable vector.
      std::vector<uchar> vec(10);
      BlobOBufVector<uchar> bob(vec,5);
      doOut (&bob);
      std::cout << "BobOBufVector bobSize=" << bob.size()
		<< " vecSize=" << vec.size()
		<< " vecCap=" << vec.capacity() << std::endl;
      BlobIBufVector<uchar> bib(vec);
      doIn (&bib);
    }
    {
      std::vector<uchar> vec;
      BlobOBufVector<uchar> bob(vec,100);
      doOut (&bob);
      std::cout << "BobOBufVector bobSize=" << bob.size()
		<< " vecSize=" << vec.size()
		<< " vecCap=" << vec.capacity() << std::endl;
      BlobIBufVector<uchar> bib(vec);
      doIn (&bib);
    }
   {
     // Use an expandable string.
     BlobString str(BlobStringType(true), 10);
     BlobOBufString bob(str,5);
     doOut (&bob);
     std::cout << "BobOBufString bobSize=" << bob.size()
	       << " strSize=" << str.size() << std::endl;
     BlobIBufString bib(str);
     doIn (&bib);
   }
   {
     BlobString str(BlobStringType(true));
     BlobOBufString bob(str, 100);
     doOut (&bob);
     std::cout << "BobOBufString bobSize=" << bob.size()
	       << " strSize=" << str.size() << std::endl;
     BlobIBufString bib(str);
     doIn (&bib);
   }
    {
      try {
	// Use a fixed char buffer which is too small.
	char buf[50];
	BlobOBufChar bob(buf,50);
	doOut (&bob);
      } catch (std::exception& x) {
	std::cout << x.what() << std::endl;
      }
    }
    {
      try {
	// Use a fixed vector buffer which is too small.
	std::vector<char> buf(50);
	BlobOBufVector<char> bob(buf);
	doOut (&bob);
      } catch (std::exception& x) {
	std::cout << x.what() << std::endl;
      }
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

