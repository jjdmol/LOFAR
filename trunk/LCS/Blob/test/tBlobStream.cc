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

  bos.putStart ("testall", 1);
  bool valbl[2];
  char valsc[2];
  uchar valuc[2];
  short valss[2];
  ushort valus[2];
  int valsi[2];
  uint valui[2];
  float valsf[2];
  double valsd[2];
  fcomplex valfc[2];
  dcomplex valdc[2];
  string valst[2];
  bool valb2[] = {false,false,true,true,true,false,true,false,true};
  valbl[0] = true;
  valbl[1] = false;
  valsc[0] = char(2);
  valsc[1] = char(-20);
  valuc[0] = uchar(1);
  valuc[1] = uchar(3);
  valss[0] = short(22);
  valss[1] = short(-220);
  valus[0] = ushort(21);
  valus[1] = ushort(23);
  valsi[0] = int(222);
  valsi[1] = int(-2220);
  valui[0] = uint(221);
  valui[1] = uint(223);
  valsf[0] = float(7.3);
  valsf[1] = float(-2.4);
  valsd[0] = double(47.3);
  valsd[1] = double(-32.4);
  valdc[0] = fcomplex(-3.4,8.7);
  valdc[1] = fcomplex(3.4,-8.7);
  valdc[0] = dcomplex(-43.4,58.7);
  valdc[1] = dcomplex(33.4,-68.7);
  valst[0] = string("0123456789");
  valst[1] = "abcdefghijklmnopqrstuvwxyz";
  bos << valbl[0];
  bos << valsc[0];
  bos << valuc[0];
  bos << valss[0];
  bos << valus[0];
  bos << valsi[0];
  bos << valui[0];
  bos << valsf[0];
  bos << valsd[0];
  bos << valfc[0];
  bos << valdc[0];
  bos << valst[0];
  bos.put (valb2, sizeof(valb2));
  bos.put (valbl, 2);
  bos.put (valsc, 2);
  bos.put (valuc, 2);
  bos.put (valss, 2);
  bos.put (valus, 2);
  bos.put (valsi, 2);
  bos.put (valui, 2);
  bos.put (valsf, 2);
  bos.put (valsd, 2);
  bos.put (valfc, 2);
  bos.put (valdc, 2);
  bos.put (valst, 2);
  ln = 233;
  if (header8) ln += 3;
  {
    std::vector<uint> veci(600);
    std::vector<bool> vecb(veci.size());
    for (uint i=0; i<veci.size(); i++) {
      veci[i] = i;
      vecb[i] = bool(i%3);
    }
    bos.put (veci);
    bos.put (vecb);
    ln += 4 + 4*veci.size() + 4 + (vecb.size()+7)/8;
    bool bufb[2007];
    for (uint i=0; i<2007; i++) {
      bufb[i] = bool(i%7);
    }
    bos.put (bufb, 2007);
    ln += (2007+7)/8;
  }
  Assert (bos.putEnd() == ln);
  len += ln;

  return len;
}

void doIn (BlobIBuffer* bb, bool header8=false)
{
  bool  valb;
  int16 val16;
  int64 val64;
  fcomplex     valfcx;
  dcomplex     valdcx;
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
  bis >> valfcx;
  Assert (valfcx == fcomplex(1,2));
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
  bis >> valdcx;
  Assert (valdcx == dcomplex(5,6));
  ln = 43;
  if (header8) ln = 48;
  Assert (bis.getEnd() == ln);

  bis.getStart ("testall");
  bool valbl[2], valblc[2];
  char valsc[2], valscc[2];
  uchar valuc[2], valucc[2];
  short valss[2], valssc[2];
  ushort valus[2], valusc[2];
  int valsi[2], valsic[2];
  uint valui[2], valuic[2];
  float valsf[2], valsfc[2];
  double valsd[2], valsdc[2];
  fcomplex valfc[2], valfcc[2];
  dcomplex valdc[2], valdcc[2];
  string valst[2], valstc[2];
  bool valb2[] = {false,false,true,true,true,false,true,false,true};
  bool valb2c[sizeof(valb2)];
  valbl[0] = true;
  valbl[1] = false;
  valsc[0] = char(2);
  valsc[1] = char(-20);
  valuc[0] = uchar(1);
  valuc[1] = uchar(3);
  valss[0] = short(22);
  valss[1] = short(-220);
  valus[0] = ushort(21);
  valus[1] = ushort(23);
  valsi[0] = int(222);
  valsi[1] = int(-2220);
  valui[0] = uint(221);
  valui[1] = uint(223);
  valsf[0] = float(7.3);
  valsf[1] = float(-2.4);
  valsd[0] = double(47.3);
  valsd[1] = double(-32.4);
  valdc[0] = fcomplex(-3.4,8.7);
  valdc[1] = fcomplex(3.4,-8.7);
  valdc[0] = dcomplex(-43.4,58.7);
  valdc[1] = dcomplex(33.4,-68.7);
  valst[0] = string("0123456789");
  valst[1] = "abcdefghijklmnopqrstuvwxyz";
  bis >> valblc[0];
  Assert (valbl[0] == valblc[0]);
  bis >> valscc[0];
  Assert (valsc[0] == valscc[0]);
  bis >> valucc[0];
  Assert (valuc[0] == valucc[0]);
  bis >> valssc[0];
  Assert (valss[0] == valssc[0]);
  bis >> valusc[0];
  Assert (valus[0] == valusc[0]);
  bis >> valsic[0];
  Assert (valsi[0] == valsic[0]);
  bis >> valuic[0];
  Assert (valui[0] == valuic[0]);
  bis >> valsfc[0];
  Assert (valsf[0] == valsfc[0]);
  bis >> valsdc[0];
  Assert (valsd[0] == valsdc[0]);
  bis >> valfcc[0];
  Assert (valfc[0] == valfcc[0]);
  bis >> valdcc[0];
  Assert (valdc[0] == valdcc[0]);
  bis >> valstc[0];
  Assert (valst[0] == valstc[0]);
  bis.get (valb2c, sizeof(valb2c));
  bis.get (valblc, 2);
  bis.get (valscc, 2);
  bis.get (valucc, 2);
  bis.get (valssc, 2);
  bis.get (valusc, 2);
  bis.get (valsic, 2);
  bis.get (valuic, 2);
  bis.get (valsfc, 2);
  bis.get (valsdc, 2);
  bis.get (valfcc, 2);
  bis.get (valdcc, 2);
  bis.get (valstc, 2);
  for (uint i=0; i<2; i++) {
    Assert (valbl[i] == valblc[i]);
    Assert (valsc[i] == valscc[i]);
    Assert (valuc[i] == valucc[i]);
    Assert (valss[i] == valssc[i]);
    Assert (valus[i] == valusc[i]);
    Assert (valsi[i] == valsic[i]);
    Assert (valui[i] == valuic[i]);
    Assert (valsf[i] == valsfc[i]);
    Assert (valsd[i] == valsdc[i]);
    Assert (valfc[i] == valfcc[i]);
    Assert (valdc[i] == valdcc[i]);
    Assert (valst[i] == valstc[i]);
  }
  for (uint i=0; i<sizeof(valb2); i++) {
    Assert (valb2[i] == valb2c[i]);
  }
  {
    std::vector<uint> veci;
    std::vector<bool> vecb;
    bis.get (veci);
    bis.get (vecb);
    Assert (veci.size() == 600);
    Assert (veci.size() == vecb.size());
    for (uint i=0; i<veci.size(); i++) {
      AssertStr (veci[i] == i, i);
      AssertStr (vecb[i] == bool(i%3), i);
    }
    bool bufb[2007];
    bis.get (bufb, 2007);
    for (uint i=0; i<2007; i++) {
      Assert (bufb[i] == bool(i%7));
    }
  }
  bis.getEnd();
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
      {
	// Use a standard little-endian file as input.
	std::ifstream is ("tBlobStream.in_le");
	BlobIBufStream bib(is);
	doIn (&bib);
	Assert (int(is.tellg()) == len);
      }
      {
	// Use a standard big-endian file as input.
	std::ifstream is ("tBlobStream.in_be");
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
      uchar buf[3500];
      BlobOBufChar bob(buf,3500);
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

