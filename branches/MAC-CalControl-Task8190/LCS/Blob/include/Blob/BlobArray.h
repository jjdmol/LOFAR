//# BlobArray.h: Blob handling for arrays
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BLOB_BLOBARRAY_H
#define LOFAR_BLOB_BLOBARRAY_H

// \file
// Blob handling for arrays

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <vector>
#if defined(HAVE_BLITZ) 
# include <blitz/array.h>
#endif
#if defined(HAVE_AIPSPP) 
# include <casa/Arrays/Array.h>
#endif


namespace LOFAR
{

// \ingroup %pkgname%

// \addtogroup BlobArray Global BlobArray functions
  // Define functions to write N-dimensional arrays into a blob and to
  // read them back from a blob.
  // The arrays can be:
  // <ul>
  // <li> A plain N-dimensional C-array.
  // <li> A blitz array in Fortran order (minor axis first) or in C order.
  // <li> An AIPS++ array (which is in Fortran order).
  // </ul>
  // Special functions exist to read or write a vector (1-dimensional array).
  // Because all array types are written in a standard way, it is possible
  // to write, for example, a blitz array and read it back as an AIPS++ array.
  // If the axes ordering is different, the axes are reversed.
  //
  // The write functions follow the same standard as the static array header
  // defined in BlobArrayHeader.h, so it is possible to read a static array
  // back in a dynamic way.
// <group>
  
  // \name General function to write a data array
  // Usually it is used by the other functions, but it can be used on
  // its own to write, say, a C-style array.
  // A 1-dim C-style array can be written with putBlobVector.
  // <group>
  template<typename T>
  BlobOStream& putBlobArray (BlobOStream& bs, const T* data,
			     const uint64* shape, uint16 ndim,
			     bool fortranOrder);
  template<typename T>
  BlobOStream& putBlobVector (BlobOStream& bs, const T* data, uint64 size);
  // </group>

  // \name Reserve space for an array with the given shape
  // The axes ordering (Fortran or C-style) has to be given.
  // It returns the offset of the array in the blob.
  // It is useful for allocating a static blob in a dynamic way.
  // It is only possible if the underlying buffer is seekable.
  // It is meant for use with the BlobOBufString buffer. The function
  // getPointer in that class can be used to turn the position into a pointer.
  // The data will be aligned on the given alignment which defaults to
  // sizeof(T) bytes with a maximum of 8.
  // <group>
  template<typename T>
  uint64 setSpaceBlobArray1 (BlobOStream& bs, bool useBlobHeader,
                             uint64 size0, uint alignment=0);
  template<typename T>
  uint64 setSpaceBlobArray2 (BlobOStream& bs, bool useBlobHeader,
                             uint64 size0, uint64 size1,
                             bool fortranOrder, uint alignment=0);
  template<typename T>
  uint64 setSpaceBlobArray3 (BlobOStream& bs, bool useBlobHeader,
                             uint64 size0, uint64 size1, uint64 size2,
                             bool fortranOrder, uint alignment=0);
  template<typename T>
  uint64 setSpaceBlobArray4 (BlobOStream& bs, bool useBlobHeader,
                             uint64 size0, uint64 size1,
                             uint64 size2, uint64 size3,
                             bool fortranOrder, uint alignment=0);
  template<typename T>
  uint64 setSpaceBlobArray (BlobOStream& bs, bool useBlobHeader,
                            const std::vector<uint64>& shape,
                            bool fortranOrder, uint alignment=0);
  template<typename T>
  uint64 setSpaceBlobArray (BlobOStream& bs, bool useBlobHeader,
                            const uint64* shape, uint16 ndim,
                            bool fortranOrder, uint alignment=0);
  // </group>


#if defined(HAVE_BLITZ) 
  // Write a blitz array (which can be non-contiguous).
  template<typename T, uint N>
  BlobOStream& operator<< (BlobOStream&, const blitz::Array<T,N>&);

  // Read back a blitz array.
  // The dimensionality found in the stream has to match N.
  // If the shape mismatches, the array is resized.
  // If the shape matches, the array can be non-contiguous.
  template<typename T, uint N>
  BlobIStream& operator>> (BlobIStream&, blitz::Array<T,N>&);
#endif

#if defined(HAVE_AIPSPP) 
  // Write an AIPS++ array (which can be non-contiguous).
  template<typename T>
  BlobOStream& operator<< (BlobOStream&, const casa::Array<T>&);

  // Read back an AIPS++ array.
  // If the shape mismatches, the array is resized.
  // If the shape matches, the array can be non-contiguous.
  template<typename T>
  BlobIStream& operator>> (BlobIStream&, casa::Array<T>&);

  // Write/read the shape of an AIPS++ array.
  // <group>
  BlobOStream& operator<< (BlobOStream&, const casa::IPosition&);
  BlobIStream& operator>> (BlobIStream&, casa::IPosition&);
  // </group>
#endif

  // \name Write a vector of objects
  // <group>
  BlobOStream& operator<< (BlobOStream&, const std::vector<bool>&);
  template<typename T>
  BlobOStream& operator<< (BlobOStream&, const std::vector<T>&);
  // </group>

  // \name Read back a vector of objects
  // The dimensionality found in the stream has to be 1.
  // The vector is resized as needed.
  // <group>
  BlobIStream& operator>> (BlobIStream&, std::vector<bool>&);
  template<typename T>
  BlobIStream& operator>> (BlobIStream&, std::vector<T>&);
  // </group>

  // Read back as a C-style vector.
  // It allocates the required storage and puts the pointer to it in arr.
  // The user is responsible for deleting the data.
  template<typename T>
  BlobIStream& getBlobVector (BlobIStream& bs, T*& arr, uint64& size);

  // Read back as a C-array with the axes in Fortran or C-style order.
  // It allocates the required storage and puts the pointer to it in arr.
  // The shape is stored in the vector.
  // The user is responsible for deleting the data.
  template<typename T>
  BlobIStream& getBlobArray (BlobIStream& bs, T*& arr,
			     std::vector<uint64>& shape,
			     bool fortranOrder);

  // Find an array in the blob with the axes in Fortran or C-style order.
  // The shape is stored in the vector.
  // It returns the offset of the array in the buffer and treats the array
  // as being read (thus skips over the data).
  // It is only possible if the underlying buffer is seekable.
  // It is meant for use with the BlobIBufString buffer. The function
  // getPointer in that class can be used to turn the position into a pointer.
  // The data are assumed to be aligned on the given alignment which
  // defaults to sizeof(T) bytes with a maximum of 8.
  template<typename T>
  uint64 getSpaceBlobArray (BlobIStream& bs, bool useBlobHeader,
                            std::vector<uint64>& shape,
                            bool fortranOrder);




  //# Reserve space for a 1-dim array of the given size.
  template<typename T>
  inline uint64 setSpaceBlobArray1 (BlobOStream& bs, bool useBlobHeader,
                                    uint64 size0, uint alignment)
  {
    return setSpaceBlobArray<T> (bs, useBlobHeader, &size0, 1, true,
				 alignment);
  }

  //# Put a vector object as an array.
  template<typename T>
  inline BlobOStream& operator<< (BlobOStream& bs, const std::vector<T>& vec)
  {
    return putBlobVector (bs, vec.empty() ? 0 : &(vec[0]), vec.size());
  }

  //# Put a C-style vector of values as an array.
  template<typename T>
  inline BlobOStream& putBlobVector (BlobOStream& bs, const T* vec,
				     uint64 size)
  {
    return putBlobArray (bs, vec, &size, 1, true);
  }


  // Put a blob array header. It returns the number of elements in the array.
  // This is a helper function for the functions writing an array.
  // After writing the shape it aligns the stream on the given alignment.
  uint64 putBlobArrayHeader (BlobOStream& bs, bool useBlobHeader,
			     const std::string& headerName,
			     const uint64* shape, uint16 ndim,
			     bool fortranOrder, uint alignment);

  // Get the ordering and dimensionality.
  // This is a helper function for the functions reading an array.
  // It returns the number of alignment bytes used.
  inline uint getBlobArrayStart (BlobIStream& bs, bool& fortranOrder,
				 uint16& ndim)
  {
    uchar nalign;
    bs >> fortranOrder >> nalign >> ndim;
    return nalign;
  }


  // Convert the array header data.
  void convertArrayHeader (LOFAR::DataFormat, char* header,
			   bool useBlobHeader);

  // Get the shape of an array from the blob.
  // This is a helper function for the functions reading an array.
  // It returns the number of elements in the array.
  uint64 getBlobArrayShape (BlobIStream& bs, uint64* shape, uint ndim,
                            bool swapAxes, uint nalign);

  // Helper function to put an array of data.
  // It is specialized for the standard types (including complex and string).
  template<typename T> void putBlobArrayData (BlobOStream& bs,
					      const T* data, uint64 nr);

  // Helper function to get an array of data.
  template<typename T> void getBlobArrayData (BlobIStream& bs,
					      T* data, uint64 nr);

  // Specializations for the standard types (including complex and string).
#define BLOBARRAY_PUTGET_SPEC(TP) \
template<> inline void putBlobArrayData (BlobOStream& bs, \
	   			         const TP* data, uint64 nr) \
  { bs.put (data, nr); } \
template<> inline void getBlobArrayData (BlobIStream& bs, \
				         TP* data, uint64 nr) \
  { bs.get (data, nr); }
BLOBARRAY_PUTGET_SPEC(bool)
BLOBARRAY_PUTGET_SPEC(int8)
BLOBARRAY_PUTGET_SPEC(uint8)
BLOBARRAY_PUTGET_SPEC(int16)
BLOBARRAY_PUTGET_SPEC(uint16)
BLOBARRAY_PUTGET_SPEC(int32)
BLOBARRAY_PUTGET_SPEC(uint32)
BLOBARRAY_PUTGET_SPEC(int64)
BLOBARRAY_PUTGET_SPEC(uint64)
BLOBARRAY_PUTGET_SPEC(float)
BLOBARRAY_PUTGET_SPEC(double)
BLOBARRAY_PUTGET_SPEC(fcomplex)
BLOBARRAY_PUTGET_SPEC(dcomplex)
BLOBARRAY_PUTGET_SPEC(std::string)

// </group>

} // end namespace LOFAR


#include <Blob/BlobArray.tcc>

using LOFAR::operator<<;
using LOFAR::operator>>;
using LOFAR::putBlobArray;
using LOFAR::putBlobVector;
using LOFAR::setSpaceBlobArray1;
using LOFAR::setSpaceBlobArray2;
using LOFAR::setSpaceBlobArray3;
using LOFAR::setSpaceBlobArray4;
using LOFAR::setSpaceBlobArray;
using LOFAR::getBlobVector;
using LOFAR::getBlobArray;
using LOFAR::getSpaceBlobArray;

#endif
