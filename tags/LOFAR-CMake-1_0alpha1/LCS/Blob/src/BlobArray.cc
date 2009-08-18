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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Blob/BlobArray.h>
#include <Blob/BlobHeader.h>
#include <vector>

namespace LOFAR
{

uint32 putBlobArrayHeader (BlobOStream& bs, bool useBlobHeader,
			   const string& headerName,
			   const uint32* shape, uint16 ndim,
			   bool fortranOrder, uint alignment)
{
  if (useBlobHeader) {
    bs.putStart (headerName, 1);                // version 1
  }
  uchar nalign = 0;
  if (alignment > 1) {
    int64 pos = bs.tellPos();
    if (pos > 0) {
      nalign = (pos + 4 + ndim*sizeof(uint32)) % alignment;
      if (nalign > 0) {
	nalign = alignment - nalign;
      }
    }
  }    
  bs << fortranOrder << nalign << ndim;
  bs.put (shape, ndim);
  uint32 n = 1;
  for (int i=0; i<ndim; i++) {
    n *= shape[i];
  }
  if (nalign > 0) {
    bs.put ("        ", nalign);
  }
  return n;
}

// Get the shape of an array from the blob.
// This is a helper function for the functions reading an array.
uint getBlobArrayShape (BlobIStream& bs, uint32* shape, uint ndim,
			bool swapAxes, uint nalign)
{
  bs.get (shape, ndim);
  if (swapAxes) {
    std::vector<uint32> shp(ndim);
    for (uint i=0; i<ndim; i++) {
      shp[i] = shape[i];
    }
    for (uint i=0; i<ndim; i++) {
      shape[i] = shp[ndim-i-1];
    }
  }
  uint n=1;
  for (uint i=0; i<ndim; i++) {
    n *= shape[i];
  }
  char buf[32];
  while (nalign > 0) {
    int nb = std::min(32u, nalign);
    bs.get (buf, nb);
    nalign -= nb;
  }
  return n;
}

void convertArrayHeader (LOFAR::DataFormat fmt, char* header,
			 bool useBlobHeader)
{
  char* buf = header;
  if (useBlobHeader) {
    BlobHeader* hdr = (BlobHeader*)header;
    hdr->setLocalDataFormat();
    buf += hdr->getHeaderLength();
  }
  // Skip the first 2 characters that do not need to be converted.
  buf += + 2;
  // Get ndim and convert it in the buffer.
  uint16 ndim;
  dataConvert16 (fmt, &ndim, buf);
  dataConvert16 (fmt, buf);
  buf += 2;
  // Convert all dimensions.
  dataConvert32 (fmt, buf, ndim);
}

BlobOStream& operator<< (BlobOStream& bs, const std::vector<bool>& vec)
{
  uint32 size = vec.size();
  putBlobArrayHeader (bs, true,
		      LOFAR::typeName((const bool**)0),
		      &size, 1, true, 1);
  bs.putBoolVec (vec);
  bs.putEnd();
  return bs;
}

BlobIStream& operator>> (BlobIStream& bs, std::vector<bool>& vec)
{
  bs.getStart (LOFAR::typeName((const bool**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  ASSERT(ndim == 1);
  uint32 size;
  getBlobArrayShape (bs, &size, 1, false, nalign);
  bs.getBoolVec (vec, size);
  return bs;
}

#if defined(HAVE_AIPSPP) 
BlobOStream& operator<< (BlobOStream& bs, const casa::IPosition& ipos)
{
  uint32 size = ipos.size();
  putBlobArrayHeader (bs, true,
		      LOFAR::typeName((const casa::Int**)0),
		      &size, 1, true, 1);
  bs.put (ipos.begin(), size);
  bs.putEnd();
  return bs;
}

BlobIStream& operator>> (BlobIStream& bs, casa::IPosition& ipos)
{
  bs.getStart (LOFAR::typeName((const casa::Int**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  ASSERT(ndim == 1);
  uint32 size;
  getBlobArrayShape (bs, &size, 1, false, nalign);
  ipos.resize (size, false);
  bs.get (ipos.begin(), size);
  bs.getEnd();
  return bs;
}
#endif

} //end namespace LOFAR
