//# BlobArray.cc: Blob handling for arrays
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
#include <Common/BlobHeader.h>
#include <vector>

namespace LOFAR
{

uint32 putBlobArrayHeader (BlobOStream& bs, bool useBlobHeader,
			   const string& headerName,
			   const uint32* shape, uint16 ndim,
			   bool fortranOrder, uint align)
{
  if (useBlobHeader) {
    bs.putStart (headerName, 1, align);                // version 1
  }
  bs << fortranOrder << char(0) << ndim;
  bs.put (shape, ndim);
  if (ndim%2 == 0) {
    bs << uint32(0);      // make #axes odd (to match BlobArrayHeader)
  }
  uint32 n = 1;
  for (int i=0; i<ndim; i++) {
    n *= shape[i];
  }
  return n;
}

// Get the shape of an array from the blob.
// This is a helper function for the functions reading an array.
void getBlobArrayShape (BlobIStream& bs, uint32* shape, uint ndim,
			bool swapAxes)
{
  bs.get (shape, ndim);
  if (ndim%2 == 0) {
    uint32 dummy;
    bs >> dummy;    // a dummy is added to make #dim odd (for BlobArrayHeader)
  }
  if (swapAxes) {
    std::vector<uint32> shp(ndim);
    for (uint i=0; i<ndim; i++) {
      shp[i] = shape[i];
    }
    for (uint i=0; i<ndim; i++) {
      shape[i] = shp[ndim-i-1];
    }
  }
}

void convertArrayHeader (LOFAR::DataFormat fmt, char* header,
			 bool useBlobHeader)
{
  char* buf = header;
  if (useBlobHeader) {
    BlobHeaderBase* hdr = (BlobHeaderBase*)header;
    hdr->setLocalDataFormat();
    buf += hdr->getHeaderLength();
  }
  // Skip the 2 characters that do not need to be converted.
  buf += + 2;
  int ndim = dataConvert (fmt, *((uint16*)buf));
  dataConvert16 (fmt, buf);
  buf += 2;
  for (int i=0; i<ndim; i++) {
    dataConvert32 (fmt, buf);
    buf += 4;
  }
}

BlobOStream& operator<< (BlobOStream& bs, const std::vector<bool>& vec)
{
  uint32 size = vec.size();
  putBlobArrayHeader (bs, true,
		      LOFAR::typeName((const bool**)0),
		      &size, 1, true);
  bs.putBoolVec (vec);
  bs.putEnd();
  return bs;
}

BlobIStream& operator>> (BlobIStream& bs, std::vector<bool>& vec)
{
  bs.getStart (LOFAR::typeName((const bool**)0));
  bool fortranOrder;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder, ndim);
  Assert (ndim == 1);
  uint32 size;
  getBlobArrayShape (bs, &size, 1, false);
  bs.getBoolVec (vec, size);
  return bs;
}

} //end namespace LOFAR
