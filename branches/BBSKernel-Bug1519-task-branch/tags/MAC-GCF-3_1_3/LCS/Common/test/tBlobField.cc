//# tBlobArray.cc: Test program for BlobFieldSet class
//#
//# Copyright (C) 2004
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

#include <Common/BlobFieldSet.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobIBufVector.h>
#include <Common/BlobHeader.h>
#include <Common/DataConvert.h>
#include <iostream>
#include <fstream>
#include <vector>

//# Needed for instantiating BlobField<XX>
#include <Common/BlobField.cc>


using namespace LOFAR;

// Define a struct XX for testing a field with a non-native type.
struct XX
{
  XX(int i, float f) : f1(i), f2(f) {};
  bool operator==(const XX& that) const { return f1==that.f1 && f2==that.f2; }
  int32 f1;
  float f2;
};

// Define the function to convert XX from given format to local format.
namespace LOFAR {
  void dataConvert (DataFormat fmt, XX* buf, uint nrval)
  {
    for (uint i=0; i<nrval ;i++) {
      dataConvert32 (fmt, &(buf[i].f1));
      dataConvertFloat (fmt, &(buf[i].f2));
    }
  }
}

// Instantiate the template.
template class BlobField<XX>;


// Function to create and fill a blob.
void doOut (BlobFieldSet& fset, BlobOBufChar& bb)
{
  // Create the blob for the given field set.
  fset.createBlob (bb);
  // Check if fields are correct.
  ASSERT (fset[0].getNelem() == 1);
  ASSERT (fset[0].isScalar());
  ASSERT (fset[1].getNelem() == 20);
  ASSERT (! fset[1].isScalar());
  ASSERT (fset[2].getNelem() == 2);
  ASSERT (! fset[2].isScalar());
  // Get pointers to the data in the buffer.
  int* pint = fset[0].getData<int> (bb);
  fcomplex* pfcomplex = fset[1].getData<fcomplex> (bb);
  XX* px = fset[2].getData<XX> (bb);
  // Put data in the buffer.
  *pint = 3;
  for (uint i=0; i<fset[1].getNelem(); i++) {
    pfcomplex[i] = fcomplex(i+1.,i+2.);
  }
  px[0] = XX(1,2);
  px[1] = XX(2,1);
}

// Function to read back and check a blob.
// The field set must have the offsets filled in, so the field set used
// in doOut can be used.
void doIn (BlobFieldSet& fset, BlobIBufChar& bb)
{
  // Check if fields are correct.
  ASSERT (fset[0].getNelem() == 1);
  ASSERT (fset[0].isScalar());
  ASSERT (fset[1].getNelem() == 20);
  ASSERT (! fset[1].isScalar());
  ASSERT (fset[2].getNelem() == 2);
  ASSERT (! fset[2].isScalar());
  // Get pointers to the data.
  const int* pint = fset[0].getData<int> (bb);
  const fcomplex* pfcomplex = fset[1].getData<fcomplex> (bb);
  const XX* px = fset[2].getData<XX> (bb);
  // Check if data matches as put in doOut.
  ASSERT (*pint == 3);
  for (uint i=0; i<fset[1].getNelem(); i++) {
    ASSERT (pfcomplex[i] == fcomplex(i+1.,i+2.));
  }
  ASSERT (px[0] == XX(1,2));
  ASSERT (px[1] == XX(2,1));
}

// Function to interpret and check a blob.
void doIn2 (BlobFieldSet& fset, BlobIBufChar& bb)
{
  bool convert = BlobFieldSet::checkHeader (bb, 0, 0, bb.size());
  fset.openBlob (bb);
  // Check if fields are correct.
  ASSERT (fset[0].getNelem() == 1);
  ASSERT (fset[0].isScalar());
  ASSERT (fset[1].getNelem() == 20);
  ASSERT (! fset[1].isScalar());
  ASSERT (fset[2].getNelem() == 2);
  ASSERT (! fset[2].isScalar());
  // Get pointers to the data.
  const int* pint = fset[0].getData<int> (bb);
  const fcomplex* pfcomplex = fset[1].getData<fcomplex> (bb);
  const XX* px = fset[2].getData<XX> (bb);
  // Check if data matches as put in doOut.
  if (convert) {
    LOFAR::DataFormat fmt =
      ((BlobHeader*)(bb.getBuffer()))->getDataFormat();
    fcomplex valfc;
    XX valxx(0,0);
    ASSERT (LOFAR::dataConvert (fmt, *pint) == 3);
    for (uint i=0; i<fset[1].getNelem(); i++) {
      LOFAR::dataConvertFloat (fmt, &valfc, pfcomplex+i, 2);
      ASSERT (valfc == fcomplex(i+1.,i+2.));
    }
    memcpy (&valxx, px, sizeof(XX));
    LOFAR::dataConvert (fmt, &valxx, 1);
    ASSERT (valxx == XX(1,2));
    memcpy (&valxx, px+1, sizeof(XX));
    LOFAR::dataConvert (fmt, &valxx, 1);
    ASSERT (valxx == XX(2,1));
  } else {
    ASSERT (*pint == 3);
    for (uint i=0; i<fset[1].getNelem(); i++) {
      ASSERT (pfcomplex[i] == fcomplex(i+1.,i+2.));
    }
    ASSERT (px[0] == XX(1,2));
    ASSERT (px[1] == XX(2,1));
  }
}

// Read back and check a blob in a file.
// Note that the blob as created in doOut is written in the file as a
// byte array packed in another blob.
void readFile (BlobFieldSet& fset, BlobFieldSet& fset2,
	       const char* fileName)
{
  // Read the blob as a byte array from the file.
  std::ifstream is (fileName);
  BlobIBufStream bibs(is);
  BlobIStream bis(bibs);
  std::vector<char> buf;
  bis >> buf;
  // Use the byte array as a buffer and check its header.
  BlobIBufVector<char> bbuf(buf);
  bool cv = BlobFieldSet::checkHeader (bbuf, "test1", 1, 0);
  // Interpret the data directly.
  BlobFieldSet fsetc(fset);
  doIn2 (fsetc, bbuf);
  bbuf.setPos (0);
  // Convert the data if needed.
  if (cv) {
    fset.convertData (bbuf);
  }
  // Read and check the blob using the normal field set.
  doIn (fset, bbuf);
  // Do the same for a field set with extra fields (version 2 fields).
  // Such fields should be ignored.
  // Open the blob again to get correct offsets in fset2.
  fset2.openBlob (bbuf);
  doIn (fset2, bbuf);  // this should ignore the double field
}

int main()
{
  try {
    INIT_LOGGER("tBlobField.log_prop");
    {
      // Create a version 1 field set.
      BlobFieldSet fset("test1");
      fset.add (BlobField<int> (1));              // scalar
      fset.add (BlobField<fcomplex> (1, 4, 5));   // 2-dim array
      fset.add (BlobField<XX> (1, 2));            // 1-dim array
      fset[0].setUseBlobHeader(true);
      fset[1].setUseBlobHeader(true);
      fset[2].setUseBlobHeader(true);
      // Make another set with an extra field (with version 2).
      BlobFieldSet fset2(fset);
      fset2.add (BlobField<double> (2));
      fset2[3].setUseBlobHeader(true);
      ASSERT (fset2.hasFixedShape());
      {
	// Create the blob and read it back.
	BlobString bstr;
	BlobOBufString bob(bstr);
	doOut (fset, bob);
	BlobIBufString bib(bstr);
	doIn (fset, bib);
	// Read it back with a newer version field set.
	fset2.openBlob (bib);
	doIn (fset2, bib);  // this should ignore the double field
	// Write the blob in a file (as an array of bytes).
	std::ofstream os ("tBlobField_tmp.dat");
	BlobOBufStream bobs(os);
	BlobOStream bos(bobs);
	putBlobVector (bos, bstr.data(), bstr.size());
      }
      // Reinterpret the blob from the file.
      readFile (fset, fset2, "tBlobField_tmp.dat");
      readFile (fset, fset2, "tBlobField.in_le");
      readFile (fset, fset2, "tBlobField.in_be");
    }
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}

