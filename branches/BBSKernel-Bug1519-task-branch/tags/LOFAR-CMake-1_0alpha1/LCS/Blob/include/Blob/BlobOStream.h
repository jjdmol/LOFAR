//# BlobOStream.h: Output stream for a blob
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

#ifndef LOFAR_BLOB_BLOBOSTREAM_H
#define LOFAR_BLOB_BLOBOSTREAM_H

// \file
// Output stream for a blob

#include <Common/LofarTypes.h>
#include <Blob/BlobOBuffer.h>
#include <stack>
#include <vector>
#include <string>
#include <complex>
#include <cstring>

namespace LOFAR {

// \ingroup %pkgname%
// <group>
  
  // This class makes it possible to create a blob.
  // It creates a header (in the putStart function) using the
  // BlobHeader definition.
  // The user can define overloaded operator<< functions to be able
  // to put objects of a given class into the blob stream.
  //
  // The blob is written into a BlobOBuffer object that can be a memory
  // buffer or an ostream object. The BlobIStream class can be used to
  // retrieve objects from a blob.
  //
// See %LOFAR document
  // <a href="http://www.lofar.org/forum/document.php?action=match&docname=LOFAR-ASTRON-MAN-006">
  // LOFAR-ASTRON-MAN-006</a> for more information.
  
  class BlobOStream
    {
    public:
      // Construct it with the underlying buffer object.
      // It keeps a pointer to the buffer, so be sure that the BlobOBuffer
      // is not deleted before this object.
      explicit BlobOStream (BlobOBuffer&);
      
      // Destructor.
      ~BlobOStream();
      
      // Clear the object. I.e., reset the current level and length.
      void clear();
      
      // Get the total size.
      uint64 size() const
	{ return itsCurLength; }
      
      // Start putting a blob. It writes the header containing data that are
      // checked when reading the blob back in BlobIStream::getStart.
      // Data in nested objects can be put without an intermediate putStart.
      // However, for complex objects it is recommended to do a putStart
      // to have a better checking.
      // <br>
      // After all values (including nested objects) of the object have
      // been put, a call to putEnd has to be done.
      //
      // If no or an empty objecttype is given, the header is
      // written without the objecttype and the associated length fields.
      // In that case the function getStart in BlobIStream should also be
      // called that way.
      //
      // The function returns the nesting level.
      // <group>
      uint putStart (int objectVersion);
      uint putStart (const std::string& objectType, int objectVersion);
      uint putStart (const char* objectType, int objectVersion);
      // </group>
      
      // End putting an object. It returns the object length (including
      // possible nested objects).
      uint putEnd();
      
      // Put a single value.
      // A string will be stored as a length followed by the characters.
      //# Note that a function is defined for the standard complex class
      //# and for the types fcomplex, dcomplex, i16complex and u16complex.
      //# This should be fine for the case where fcomplex is the builtin
      //# complex type and the case where it is the standard class.
      //# In the first case the fcomplex function is a separate function,
      //# in the second case a specialisation of the templated function.
      // <group>
      BlobOStream& operator<< (const bool& value);
      BlobOStream& operator<< (const char& value);
      BlobOStream& operator<< (const uchar& value);
      BlobOStream& operator<< (const int16& value);
      BlobOStream& operator<< (const uint16& value);
      BlobOStream& operator<< (const int32& value);
      BlobOStream& operator<< (const uint32& value);
      BlobOStream& operator<< (const int64& value);
      BlobOStream& operator<< (const uint64& value);
      BlobOStream& operator<< (const float& value);
      BlobOStream& operator<< (const double& value);
      template<class T> BlobOStream& operator<< (const std::complex<T>& value);
      BlobOStream& operator<< (const i4complex& value);
      BlobOStream& operator<< (const i16complex& value);
      BlobOStream& operator<< (const u16complex& value);
      BlobOStream& operator<< (const fcomplex& value);
      BlobOStream& operator<< (const dcomplex& value);
      BlobOStream& operator<< (const std::string& value);
      BlobOStream& operator<< (const char* value);
      // </group>
      
      // Put an array of values with the given number of values.
      // Bool values are stored as bits.
      // <group>
      void put (const bool* values, uint nrval);
      void put (const char* values, uint nrval);
      void put (const uchar* values, uint nrval);
      void put (const int16* values, uint nrval);
      void put (const uint16* values, uint nrval);
      void put (const int32* values, uint nrval);
      void put (const uint32* values, uint nrval);
      void put (const int64* values, uint nrval);
      void put (const uint64* values, uint nrval);
      void put (const float* values, uint nrval);
      void put (const double* values, uint nrval);
      template<class T> void put (const std::complex<T>* values, uint nrval);
      void put (const i4complex* values, uint nrval);
      void put (const i16complex* values, uint nrval);
      void put (const u16complex* values, uint nrval);
      void put (const fcomplex* values, uint nrval);
      void put (const dcomplex* values, uint nrval);
      void put (const std::string* values, uint nrval);
      // </group>
      
      // Put a vector of values. First it puts the size of the vector.
      // Specialise for bool because a vector of bools uses bits.
      // <group>
      void put (const std::vector<bool>& values);
      template<class T> void put (const std::vector<T>& values);
      // </group>
      
      // Put a vector of bools (without putting the size).
      void putBoolVec (const std::vector<bool>& values);
      
      // Reserve the given amount of space (the opposite of
      // BlobIStream::getSpace).
      // This is useful when creating a static blob in a dynamic way.
      // It returns the position of the skipped space in the stream.
      // It is meant for use with the BlobOBufString buffer. The function
      // getPointer in that class (in fact, in its base class BlobOBufChar)
      // can be used to turn the position into a pointer.
      int64 setSpace (uint nbytes);
      
      // Add filler bytes as needed to make the total length a multiple of n.
      // In this way the next data are aligned properly.
      // It returns the number of filler bytes used.
      // It is only useful for seekable buffers.
      uint align (uint n);
      
      // Get the current stream position.
      // It returns -1 if the stream is not seekable.
      int64 tellPos() const;
      
    private:
      // Function to do the actual putStart.
      uint doPutStart (const char* objectType, uint nrc, int objectVersion);
      
      // Write the buffer, increment itsCurLength, and check
      // if everything is written.
      void putBuf (const void* buf, uint sz);
      
      // Throw an exception if a put cannot be done.
      // <group>
      void checkPut() const;
      void throwPut() const;
      // </group>
      
      
      bool   itsSeekable;
      uint32 itsCurLength;
      uint   itsLevel;
      // Object length at each level
      std::stack<uint32> itsObjLen;
      // Offset of length at each level
      std::stack<int64>  itsObjPtr;
      // The underlying stream object.
      BlobOBuffer*       itsStream;
    };
  
// </group>
  
  inline void BlobOStream::clear()
    {
      itsCurLength = 0;
      itsLevel     = 0;
    }
  
  inline uint BlobOStream::putStart (int objectVersion)
    { return doPutStart ("", 0, objectVersion); }
  
  inline uint BlobOStream::putStart (const std::string& objectType,
				     int objectVersion)
    { return doPutStart (objectType.data(), objectType.size(), objectVersion); }
  
  inline uint BlobOStream::putStart (const char* objectType,
				     int objectVersion)
    { return doPutStart (objectType, strlen(objectType), objectVersion); }
  
  inline int64 BlobOStream::tellPos() const
    { return itsStream->tellPos(); }
  
  template<class T>
    inline BlobOStream& BlobOStream::operator<< (const std::complex<T>& value)
    {
      putBuf (&value, sizeof(value));
      return *this;
    }
  template<class T>
    inline void BlobOStream::put (const std::complex<T>* values, uint nrval)
    {
      putBuf (values, nrval*sizeof(std::complex<T>));
    }

  template<class T>
    inline void BlobOStream::put (const std::vector<T>& vec)
    {
      operator<< (uint32(vec.size()));
      put (&vec[0], vec.size());
    }
  inline void BlobOStream::put (const std::vector<bool>& vec)
    {
      operator<< (uint32(vec.size()));
      putBoolVec (vec);
    }
  
  inline void BlobOStream::checkPut() const
    {
  if (itsLevel == 0) throwPut();
    }
  

} // end namespace

#endif
