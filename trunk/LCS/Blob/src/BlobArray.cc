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

void convertArrayHeader (LOFAR::DataFormat fmt, char* header)
{
  BlobHeaderBase* hdr = (BlobHeaderBase*)header;
  hdr->setLocalDataFormat();
  char* buf = header + hdr->getHeaderLength() + 2;
  int ndim = dataConvert (fmt, *((uint16*)buf));
  dataConvert16 (fmt, buf);
  buf += 2;
  for (int i=0; i<ndim; i++) {
    dataConvert32 (fmt, buf);
    buf += 4;
  }
}

} //end namespace LOFAR
