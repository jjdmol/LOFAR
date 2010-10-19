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

int main (int argc, const char* argv[])
{
  Debug::initLevels (argc, argv);
  // Create a header for a C-order array.
  BlobArrayHeader<fcomplex,4> bl(false);
  cout << sizeof(bl) << endl;
  // Check various things.
  Assert (sizeof(bl) % 8 == 0);
  Assert (bl.plainSize() == 14);
  Assert (bl.lengthOffset() == 4);
  Assert (!bl.mustConvert());
  Assert (bl.checkMagicValue());
  Assert (bl.checkType("array<fcomplex>"));
  Assert (bl.getLength() == 0);
  // Set an arbitrary length and check it.
  bl.setLength (100);
  Assert (bl.getLength() == 100);
  Assert (!bl.isFortranOrder());
  // Define a shape and check if the length is correct (including end-of-blob).
  bl.setShape (11,12,13,14);
  Assert (bl.getAxisSize(0) == 11);
  Assert (bl.getAxisSize(1) == 12);
  Assert (bl.getAxisSize(2) == 13);
  Assert (bl.getAxisSize(3) == 14);
  Assert (bl.getLength() == sizeof(bl) + 11*12*13*14*sizeof(fcomplex) +
	  sizeof(uint32));
  {
    // Check if a static BlobArray can be read dynamically.
    // Define a struct containing a blob (header, array and eob).
    struct XA {
      XA() : hdr(false), eob(BlobHeaderBase::eobMagicValue()) {}
      BlobArrayHeader<float,2> hdr;
      float data[20][15];
      uint32 eob;
    };
    // Make an object and define the array shape in the header.
    XA xa;
    xa.hdr.setShape (20,15);
    Assert (xa.hdr.getLength() == sizeof(xa));
    // Fill the array.
    int k=0;
    for (int i=0; i<20; i++) {
      for (int j=0; j<15; j++) {
	xa.data[i][j] = k++;
      }
    }
    // Copy the object to a string.
    // Check if it can be interpreted correctly as a blob.
    std::string xastr((const char*)&xa, sizeof(xa));
    std::istringstream ss(xastr);
    BlobIBufStream bib(ss);
    BlobIStream bs(bib);
    float* data;
    std::vector<uint> shape;
    // Get the array from the blob (shape and data) and check it.
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
    delete [] data;
  }  

  cout << "OK" << endl;
  return 0;
}
