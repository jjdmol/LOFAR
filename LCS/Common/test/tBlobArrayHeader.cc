//# tBlobArrayHeader.cc: Test program for class BlobArrayHeader
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

#include <Common/BlobArrayHeader.h>
#include <Common/BlobArray.h>
#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/Debug.h>
#include <iostream>
#include <vector>

using namespace LOFAR;

int main(int argc, const char* argv[])
{
  Debug::initLevels (argc, argv);
  BlobArrayHeader<fcomplex,4> bl(false);
  cout << sizeof(bl) << endl;
  Assert (sizeof(bl) % 8 == 0);
  Assert (bl.plainSize() == 14);
  Assert (bl.lengthOffset() == 4);
  Assert (!bl.mustConvert());
  Assert (bl.checkMagicValue());
  Assert (bl.checkType("array<fcomplex>"));
  Assert (bl.getLength() == 0);
  bl.setLength (100);
  Assert (bl.getLength() == 100);
  Assert (!bl.isFortranOrder());
  bl.setShape (11,12,13,14);
  Assert (bl.getAxisSize(0) == 11);
  Assert (bl.getAxisSize(1) == 12);
  Assert (bl.getAxisSize(2) == 13);
  Assert (bl.getAxisSize(3) == 14);
  Assert (bl.getLength() == sizeof(bl) + 11*12*13*14*sizeof(fcomplex));
  {
    // Check if a static BlobArray can be read dynamically.
    struct XA {
      XA() : hdr(false) {}
      BlobArrayHeader<float,2> hdr;
      float data[20][15];
    };
    XA xa;
    xa.hdr.setShape (20,15);
    Assert (xa.hdr.getLength() == sizeof(xa));
    int k=0;
    for (int i=0; i<20; i++) {
      for (int j=0; j<15; j++) {
	xa.data[i][j] = k++;
      }
    }
    std::string xastr((const char*)&xa, sizeof(xa));
    std::istringstream ss(xastr);
    BlobIBufStream bib(ss);
    BlobIStream bs(&bib);
    float* data;
    std::vector<uint> shape;
    getBlobArray (bs, data, shape, false);
    Assert (shape.size() == 2);
    Assert (shape[0]==20  &&  shape[1]==15);
    for (int i=0; i<15*20; i++) {
      Assert (data[i] == i);
    }

    // Check if a copy of the blob is correct.
    XA xb(xa);
    Assert (!xb.hdr.mustConvert());
    Assert (xb.hdr.checkMagicValue());
    Assert (xb.hdr.checkArrayType());
    Assert (xb.hdr.checkNdim());
    Assert (!xb.hdr.isFortranOrder());
    Assert (xb.hdr.getAxisSize(0) == 20);
    Assert (xb.hdr.getAxisSize(1) == 15);
    Assert (xb.hdr.getLength() == sizeof(XA));
    k=0;
    for (int i=0; i<20; i++) {
      for (int j=0; j<15; j++) {
	Assert (xb.data[i][j] == k++);
      }
    }
  }  

  cout << "OK" << endl;
  return 0;
}
