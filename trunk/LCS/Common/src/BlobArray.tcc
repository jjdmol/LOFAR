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

#ifndef COMMON_BLOBARRAY_TCC
#define COMMON_BLOBARRAY_TCC

#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/TypeNames.h>
#include <Common/Debug.h>

namespace LOFAR
{

template<typename T>
BlobOStream& putBlobArray (BlobOStream& bs, const T* data, uint16 ndim,
	                   uint32* shape, bool fortranOrder)
{
  bs.putStart (LOFAR::typeName((const T**)0), 1);     // version 1
  bs << fortranOrder << char(0) << ndim;
  bs.put (shape, ndim);
  if (ndim%2 == 0) {
    bs << uint32(0);      // make #axes odd (to match BlobArrayHeader)
  }
  uint32 n = 1;
  for (int i=0; i<ndim; i++) {
    n *= shape[i];
  }
  putBlobArrayData (bs, data, n);
  bs.putEnd();
  return bs;
}


template<typename T>
uint reserveBlobArray2 (BlobOStream& bs, uint32 size0, uint32 size1,
                        bool fortranOrder)
{
  uint32 shp[2];
  shp[0] = size0;
  shp[1] = size1;
  return reserveBlobArray<T> (bs, shp, 2, fortranOrder);
}
template<typename T>
uint reserveBlobArray3 (BlobOStream& bs, uint32 size0, uint32 size1,
                        uint32 size2, bool fortranOrder)
{
  uint32 shp[3];
  shp[0] = size0;
  shp[1] = size1;
  shp[2] = size2;
  return reserveBlobArray<T> (bs, shp, 3, fortranOrder);
}
template<typename T>
uint reserveBlobArray4 (BlobOStream& bs, uint32 size0, uint32 size1,
                        uint32 size2, uint32 size3, bool fortranOrder)
{
  uint32 shp[4];
  shp[0] = size0;
  shp[1] = size1;
  shp[2] = size2;
  shp[3] = size3;
  return reserveBlobArray<T> (bs, shp, 4, fortranOrder);
}
template<typename T>
uint reserveBlobArray (BlobOStream& bs, const std::vector<uint32>& shape,
                       bool fortranOrder)
{
  return reserveBlobArray<T> (bs, &shape[0], shape.size(), fortranOrder);
}

template<typename T>
uint reserveBlobArray (BlobOStream& bs, uint32* shape, uint16 ndim,
                       bool fortranOrder)
{
  bs.putStart (LOFAR::typeName((const T**)0), 1);     // version 1
  bs << fortranOrder << char(0) << ndim;
  bs.put (shape, ndim);
  if (ndim%2 == 0) {
    bs << uint32(0);      // make #axes odd (to match BlobArrayHeader)
  }
  uint32 n = 1;
  for (int i=0; i<ndim; i++) {
    n *= shape[i];
  }
  uint pos = bs.reserve (n*sizeof(T));
  bs.putEnd();
  return pos;
}


#if defined(HAVE_BLITZ) 
template<typename T, uint NDIM>
BlobOStream& operator<< (BlobOStream& bs, const blitz::Array<T,NDIM>& arr)
{
  if (arr.isStorageContiguous()) {
    putBlobArray (bs, arr.dataFirst(), NDIM, arr.shape().data(),
                  arr.isMinorRank(0));
  } else {
    blitz::Array<T,N> arrc(arr.copy());
    putBlobArray (bs, arrc.dataFirst(), NDIM, arrc.shape().data(),
                  arrc.isMinorRank(0));
  }
  return bs;
}

template<typename T, uint NDIM>
BlobIStream& operator>> (BlobIStream& bs, blitz::array<T,NDIM>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder, ndim);
  Assert (ndim == NDIM);
  TinyVector<NDIM>(uint32,NDIM) shape;
  getBlobArrayShape (bs, shape.data(), NDIM, fortranOrder!=arr.isMinorRank());
  arr.resize (shape);
  if (arr.isStorageContiguous()) {
    getBlobArrayData (bs, arr.dataFirst(), arr.numElements());
  } else {
    blitz::array<T,NDIM>& arrc(shape);
    getBlobArrayData (bs, arrc.dataFirst(), arrc.numElements());
    arr = arrc;
  }
  bs.getEnd();
  return bs;
}
#endif


#if defined(HAVE_AIPSPP) 
template<typename T>
BlobOStream& operator<< (BlobOStream& bs, const Array<T>& arr)
{
  bool deleteIt;
  const T* data = arr.getStorage(deleteIt);
  putBlobArray (bs, data, arr.ndim(), arr.shape().data(), true);
  arr.freeStorage (data, deleteIt);
  return bs;
}

template<typename T>
BlobIStream& operator>> (BlobIStream& bs, Array<T>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder, ndim);
  IPosition shape(ndim);
  getBlobArrayShape (bs, shape.data(), ndim, !fortranOrder);
  arr.resize (shape);
  bool deleteIt;
  T* data = arr.getStorage(deleteIt);
  getBlobArrayData (bs, data, arr.nelements());
  arr.putStorage (data, deleteIt);
  bs.getEnd();
  return bs;
}
#endif


template<typename T>
BlobIStream& getBlobVector (BlobIStream& bs, T*& arr, uint32& size)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder, ndim);
  Assert (ndim == 1);
  getBlobArrayShape (bs, &size, 1, false);
  arr = new T[size];
  getBlobArrayData (bs, arr, size);
  bs.getEnd();
  return bs;
}

template<typename T>
BlobIStream& getBlobArray (BlobIStream& bs, T*& arr,
	                   std::vector<uint32>& shape, bool fortranOrder)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder1;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder1, ndim);
  shape.resize (ndim);
  getBlobArrayShape (bs, &shape[0], ndim, fortranOrder!=fortranOrder1);
  uint n=1;
  for (uint i=0; i<ndim; i++) {
    n *= shape[i];
  }
  arr = new T[n];
  getBlobArrayData (bs, arr, n);
  bs.getEnd();
  return bs;
}

template<typename T>
uint findBlobArray (BlobIStream& bs,
	            std::vector<uint32>& shape, bool fortranOrder)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder1;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder1, ndim);
  shape.resize (ndim);
  getBlobArrayShape (bs, &shape[0], ndim, fortranOrder!=fortranOrder1);
  uint n=1;
  for (uint i=0; i<ndim; i++) {
    n *= shape[i];
  }
  uint pos = bs.skip (n*sizeof(T));
  bs.getEnd();
  return pos;
}

template<typename T>
BlobIStream& operator>> (BlobIStream& bs, std::vector<T>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  getBlobArrayStart (bs, fortranOrder, ndim);
  Assert (ndim == 1);
  uint32 size;
  getBlobArrayShape (bs, &size, 1, false);
  arr.resize (size);
  getBlobArrayData (bs, &(arr[0]), size);
  bs.getEnd();
  return bs;
}


template<typename T>
void putBlobArrayData (BlobOStream& bs, const T* data, uint nr)
{
  for (uint i=0; i<nr; i++) {
    bs << data[i];
  }
}

template<typename T>
void getBlobArrayData (BlobIStream& bs, T* data, uint nr)
{
  for (uint i=0; i<nr; i++) {
    bs >> data[i];
  }
}

} // end namespace LOFAR

#endif
