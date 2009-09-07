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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobOBufChar.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobIBufVector.h>
#include <Blob/BlobOBufVector.h>
#include <Blob/BlobIBufString.h>
#include <Blob/BlobOBufString.h>
#include <Blob/BlobOBufNull.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace LOFAR;

// Do alignment in an output stream.
void doAlign (BlobOStream& bos, uint& ln)
{
  ln += bos.align(8);
}
// Do alignment in an input stream.
void doAlign (BlobIStream& bis, uint& ln)
{
  ln += bis.align(8);
}

// Create a blob in the given buffer.
// If header8 is true, all data is aligned on 8 bytes.
// Put all possible types in the blob (both scalars and vectors).
// Always check if the length (including end-of-blob) is as expected.
int doOut (BlobOBuffer& bb, bool header8=false)
{
  uint len = 0;
  BlobOStream bos(bb);
  ASSERT (bos.putStart ("test", 1) == 1);
  uint ln = 20;
  if (header8) doAlign(bos,ln);
  bos << true;
  bos << "abc";
  bos << int64(4);
  bos.put ("defg", 4);
  ln += 1 + 8+3 + 8 + 4 + 4;      // last 4 are for the eob magic value
  ASSERT (bos.putEnd() == ln);
  len += ln;

  if (header8) doAlign(bos,len);
  ASSERT (bos.putStart ("test1", 1) == 1);
  ln = 21;
  if (header8) doAlign(bos,ln);
  bos << int16(2);
  bos << makefcomplex(1,2);
  ln += 10;
  if (header8) doAlign(bos,ln);
  ASSERT (bos.putStart ("test1a", 3) == 2);
  uint ln2 = 22;
  if (header8) doAlign(bos,ln2);
  bos << std::string("defg");
  ln2 += 12 + 4;
  ASSERT (bos.putEnd() == ln2);
  ln += ln2 + 4;
  ASSERT (bos.putEnd() == ln);
  len += ln;

  if (header8) doAlign(bos,len);
  ASSERT (bos.putStart ("test2", -1) == 1);
  ln = 21;
  if (header8) doAlign(bos,ln);
  bos << int64(100);
  bos << makedcomplex(5,6);
  ln += 28;
  ASSERT (bos.putEnd() == ln);
  len += ln;

  if (header8) doAlign(bos,len);
  bos.putStart ("testall", 1);
  ln = 23;
  if (header8) doAlign(bos,ln);
  bool valbl[2];
  int8 valsc[2];
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
  valsc[0] = int8(2);
  valsc[1] = int8(-20);
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
  valfc[0] = makefcomplex(0,0);
  valfc[1] = makefcomplex(0,0);
  valdc[0] = makedcomplex(-43.4,58.7);
  valdc[1] = makedcomplex(33.4,-68.7);
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
  ln += 1+1+1+2+2+4+4+4+8+8+16 + 8+10;
  bos.put (valb2, sizeof(valb2));     // uses 2 bytes
  bos.put (valbl, 2);                 // uses 1 byte
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
  ln += 2+1+2+2+4+4+8+8+8+16+16+32 + 8+10 + 8+26;
  {
    // Do a long vector of bools, because they are stored as bits.
    std::vector<uint> veci(600);
    std::vector<bool> vecb(veci.size());
    for (uint i=0; i<veci.size(); i++) {
      veci[i] = i;
      vecb[i] = bool(i%3);
    }
    bos.put (veci);
    bos.put (vecb);
    ln += 8 + 4*veci.size() + 8 + (vecb.size()+7)/8;
    bool bufb[2007];
    for (uint i=0; i<2007; i++) {
      bufb[i] = bool(i%7);
    }
    bos.put (bufb, 2007);
    ln += (2007+7)/8;
  }
  ln += 4;
  ASSERT (bos.putEnd() == ln);
  len += ln;

  return len;
}

// Read back the blob from the buffer and check if all data in it matches
// the value given in doOut.
// Again, align if needed.
void doIn (BlobIBuffer& bb, bool header8=false)
{
  uint dumlen;
  bool  valb;
  int16 val16;
  int64 val64;
  fcomplex     valfcx;
  dcomplex     valdcx;
  std::string  vals;

  BlobIStream bis(bb);
  ASSERT (bis.getStart ("test") == 1);
  uint ln = 20;
  if (header8) doAlign(bis,ln);
  bis >> valb;
  ASSERT (valb == true);
  bis >> vals;
  ASSERT (vals.size() == 3  &&  vals == "abc");
  bis >> vals;
  ASSERT (vals.size() == 4  &&  vals == "defg");
  ln += 28;
  ASSERT (bis.getEnd() == ln);

  if (header8) doAlign(bis,dumlen);
  ASSERT (bis.getStart ("test1") == 1);
  ln = 21;
  if (header8) doAlign(bis,ln);
  bis >> val16;
  ASSERT (val16 == 2);
  bis >> valfcx;
  ASSERT (valfcx == makefcomplex(1,2));
  ln += 10;
  if (header8) doAlign(bis,ln);
  ASSERT (bis.getStart ("test1a") == 3);
  uint ln2 = 22;
  if (header8) doAlign(bis,ln2);
  bis >> vals;
  ASSERT (vals.size() == 4  &&  vals == "defg");
  ln2 += 12 + 4;
  ASSERT (bis.getEnd() == ln2);
  ln += ln2 + 4;
  ASSERT (bis.getEnd() == ln);

  if (header8) doAlign(bis,dumlen);
  ASSERT (bis.getStart ("test2") == -1);
  ln = 21;
  if (header8) doAlign(bis,ln);
  bis >> val64;
  ASSERT (val64 == 100);
  bis >> valdcx;
  ASSERT (valdcx == makedcomplex(5,6));
  ln += 28;
  ASSERT (bis.getEnd() == ln);

  if (header8) doAlign(bis,dumlen);
  bis.getStart ("testall");
  ln = 23;
  if (header8) doAlign(bis,ln);
  bool valbl[2], valblc[2];
  int8 valsc[2], valscc[2];
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
  valsc[0] = int8(2);
  valsc[1] = int8(-20);
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
  valfc[0] = makefcomplex(0,0);
  valfc[1] = makefcomplex(0,0);
  valdc[0] = makedcomplex(-43.4,58.7);
  valdc[1] = makedcomplex(33.4,-68.7);
  valst[0] = string("0123456789");
  valst[1] = "abcdefghijklmnopqrstuvwxyz";
  bis >> valblc[0];
  ASSERT (valbl[0] == valblc[0]);
  bis >> valscc[0];
  ASSERT (valsc[0] == valscc[0]);
  bis >> valucc[0];
  ASSERT (valuc[0] == valucc[0]);
  bis >> valssc[0];
  ASSERT (valss[0] == valssc[0]);
  bis >> valusc[0];
  ASSERT (valus[0] == valusc[0]);
  bis >> valsic[0];
  ASSERT (valsi[0] == valsic[0]);
  bis >> valuic[0];
  ASSERT (valui[0] == valuic[0]);
  bis >> valsfc[0];
  ASSERT (valsf[0] == valsfc[0]);
  bis >> valsdc[0];
  ASSERT (valsd[0] == valsdc[0]);
  bis >> valfcc[0];
  ASSERT (valfc[0] == valfcc[0]);
  bis >> valdcc[0];
  ASSERT (valdc[0] == valdcc[0]);
  bis >> valstc[0];
  ASSERT (valst[0] == valstc[0]);
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
    ASSERT (valbl[i] == valblc[i]);
    ASSERT (valsc[i] == valscc[i]);
    ASSERT (valuc[i] == valucc[i]);
    ASSERT (valss[i] == valssc[i]);
    ASSERT (valus[i] == valusc[i]);
    ASSERT (valsi[i] == valsic[i]);
    ASSERT (valui[i] == valuic[i]);
    ASSERT (valsf[i] == valsfc[i]);
    ASSERT (valsd[i] == valsdc[i]);
    ASSERT (valfc[i] == valfcc[i]);
    ASSERT (valdc[i] == valdcc[i]);
    ASSERT (valst[i] == valstc[i]);
  }
  for (uint i=0; i<sizeof(valb2); i++) {
    ASSERT (valb2[i] == valb2c[i]);
  }
  {
    std::vector<uint> veci;
    std::vector<bool> vecb;
    bis.get (veci);
    bis.get (vecb);
    ASSERT (veci.size() == 600);
    ASSERT (veci.size() == vecb.size());
    for (uint i=0; i<veci.size(); i++) {
      ASSERTSTR (veci[i] == i, i);
      ASSERTSTR (vecb[i] == bool(i%3), i);
    }
    bool bufb[2007];
    bis.get (bufb, 2007);
    for (uint i=0; i<2007; i++) {
      ASSERT (bufb[i] == bool(i%7));
    }
  }
  bis.getEnd();
}


int main()
{
  try {
    INIT_LOGGER("tBlobStream");
    {
      int len;
      {
	// Use a file output stream.
	std::ofstream os ("tBlobStream_tmp.dat");
	ASSERTSTR (os, "tBlobStream_tmp.dat could not be created");
	BlobOBufStream bob(os);
        len = doOut (bob);
	ASSERT (int(os.tellp()) == len);
      }
      {
	// Use the file as input.
	std::ifstream is ("tBlobStream_tmp.dat");
	ASSERTSTR (is, "tBlobStream_tmp.dat not found");
	BlobIBufStream bib(is);
	doIn (bib);
	ASSERT (int(is.tellg()) == len);
      }
      {
	// Use a standard little-endian file as input.
	std::ifstream is ("tBlobStream.in_le");
	ASSERTSTR (is, "tBlobStream.in_le not found");
	BlobIBufStream bib(is);
	doIn (bib);
	ASSERT (int(is.tellg()) == len);
      }
      {
	// Use a standard big-endian file as input.
	std::ifstream is ("tBlobStream.in_be");
	ASSERTSTR (is, "tBlobStream.in_be not found");
	BlobIBufStream bib(is);
	doIn (bib);
	ASSERT (int(is.tellg()) == len);
      }
    }
    {
      // Use a string output stream.
      std::ostringstream os;
      BlobOBufStream bob(os);
      doOut (bob);
      // Reuse as input.
      std::istringstream is(os.str());
      BlobIBufStream bib(is);
      doIn (bib);
      // Reuse as a char input.
      std::string str(os.str());
      BlobIBufChar bibc(str.data(), str.size());
      doIn (bibc);
      // Copy to vector and use that.
      std::vector<uchar> vec(os.str().size());
      BlobIBufVector<uchar> bibv(vec);
      memcpy (&vec[0], os.str().data(), os.str().size());
      doIn (bibv);
    }
    {
      // Use a null buffer.
      BlobOBufNull bob;
      doOut (bob);
    }
    {
      // Use an expandable char buffer.
      BlobOBufChar bob(10,5);
      doOut (bob);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (bib);
    }
    {
      // Use an expandable char buffer; make header multiple of 8.
      BlobOBufChar bob(10,5);
      doOut (bob, true);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (bib, true);
    }
    {
      // Use an expandable unallocated char buffer.
      BlobOBufChar bob(0,0,10);
      doOut (bob);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (bib);
    }
    {
      // Use an unexpandable preallocated char buffer.
      uchar buf[3500];
      BlobOBufChar bob(buf,3500);
      doOut (bob);
      ASSERT (bob.getBuffer() == buf);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (bib);
    }
    {
      // Use an expandable preallocated char buffer.
      uchar buf[20];
      BlobOBufChar bob(buf,20,20);
      doOut (bob);
      ASSERT (bob.getBuffer() != buf);
      BlobIBufChar bib(bob.getBuffer(), bob.size());
      doIn (bib);
    }
    {
      // Use an expandable vector.
      std::vector<uchar> vec(10);
      BlobOBufVector<uchar> bob(vec,5);
      doOut (bob);
      std::cout << "BobOBufVector bobSize=" << bob.size()
		<< " vecSize=" << vec.size()
		<< " vecCap=" << vec.capacity() << std::endl;
      BlobIBufVector<uchar> bib(vec);
      doIn (bib);
    }
    {
      std::vector<uchar> vec;
      BlobOBufVector<uchar> bob(vec,100);
      doOut (bob);
      std::cout << "BobOBufVector bobSize=" << bob.size()
		<< " vecSize=" << vec.size()
		<< " vecCap=" << vec.capacity() << std::endl;
      BlobIBufVector<uchar> bib(vec);
      doIn (bib);
    }
   {
     // Use an expandable string.
     BlobString str(BlobStringType(true), 10);
     BlobOBufString bob(str,5);
     doOut (bob);
     std::cout << "BobOBufString bobSize=" << bob.size()
	       << " strSize=" << str.size() << std::endl;
     BlobIBufString bib(str);
     doIn (bib);
   }
   {
     BlobString str(BlobStringType(true));
     BlobOBufString bob(str, 100);
     doOut (bob);
     std::cout << "BobOBufString bobSize=" << bob.size()
	       << " strSize=" << str.size() << std::endl;
     BlobIBufString bib(str);
     doIn (bib);
   }
    {
      try {
	// Use a fixed char buffer which is too small, thus an exception.
	char buf[50];
	BlobOBufChar bob(buf,50);
	doOut (bob);
      } catch (std::exception& x) {
	std::cout << x.what() << std::endl;
      }
    }
    {
      try {
	// Use a fixed vector buffer which is too small, thus an exception.
	std::vector<char> buf(50);
	BlobOBufVector<char> bob(buf);
	doOut (bob);
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

