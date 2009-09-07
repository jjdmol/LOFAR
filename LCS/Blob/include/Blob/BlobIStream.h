//# BlobIStream.h: Input stream for a blob
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

#ifndef LOFAR_BLOB_BLOBISTREAM_H
#define LOFAR_BLOB_BLOBISTREAM_H

// \file
// Input stream for a blob

#include <Common/LofarTypes.h>
#include <Common/DataFormat.h>
#include <Common/DataConvert.h>
#include <Blob/BlobIBuffer.h>
#include <stack>
#include <vector>
#include <string>
#include <complex>

namespace LOFAR {

// \ingroup %pkgname%
// <group>
  
  // This class makes it possible to interpret a blob and create the objects
  // stored in it.
  // It reads a header (in the getStart function) using the
  // BlobHeader definition, so a static blob can also be read back.
  // The user can define overloaded operator>> functions to be able
  // to read objects of a given class from the blob stream.
  //
  // The blob is read from a BlobIBuffer object that can be a memory
  // buffer or an istream object. The BlobOStream class can be used to
  // store objects into a blob.
  //
// See %LOFAR document
  // <a href="http://www.lofar.org/forum/document.php?action=match&docname=LOFAR-ASTRON-MAN-006">
  // LOFAR-ASTRON-MAN-006</a> for more information.
  
  class BlobIStream
    {
    public:
      // Construct it with the underlying buffer object.
      // It keeps the pointer to the buffer, so be sure that the BlobIBuffer
      // is not deleted before this object.
      explicit BlobIStream (BlobIBuffer&);
      
      // Destructor.
      ~BlobIStream();
      
      // Clear the object. I.e., reset the current level and position.
      void clear();
      
      // Start getting a blob which reads the header abd checks if its type
      // matches the given object type.
      // It is the counterpart of BlobOStream::putStart.
      // <br>
      // After all values (inclusing nested objects) of the object have
      // been obtained (using operator>>), a call to getEnd has to be done.
      //
      // The getStart function returns the blob version.
      // <group>
      int getStart (const std::string& objectType);
      int getStart (const char* objectType = "");
      // </group>
      
      // End getting an object. It returns the object length (including
      // possible nested objects).
      // It checks if the entire object has been read (to keep the data
      // stream in sync). If not, an exception is thrown.
      uint64 getEnd();
      
      // getNextType gets the object type of the next piece of
      // information to read.
      // It checks if it finds the correct magic value preceeding
      // the object type.
      // The second version also returns the size of this next blob.
      // <group>
      const std::string& getNextType();
      const std::string& getNextType(uint64& size);
      // </group>
      
      // Get a single value.
      // A string is stored as a length followed by the characters.
      // <group>
      BlobIStream& operator>> (bool& value);
      BlobIStream& operator>> (char& value);
      BlobIStream& operator>> (int8& value);
      BlobIStream& operator>> (uint8& value);
      BlobIStream& operator>> (int16& value);
      BlobIStream& operator>> (uint16& value);
      BlobIStream& operator>> (int32& value);
      BlobIStream& operator>> (uint32& value);
      BlobIStream& operator>> (int64& value);
      BlobIStream& operator>> (uint64& value);
      BlobIStream& operator>> (float& value);
      BlobIStream& operator>> (double& value);
      template<class T> BlobIStream& operator>> (std::complex<T>& value);
      BlobIStream& operator>> (i4complex& value);
      BlobIStream& operator>> (i16complex& value);
      BlobIStream& operator>> (u16complex& value);
      BlobIStream& operator>> (fcomplex& value);
      BlobIStream& operator>> (dcomplex& value);
      BlobIStream& operator>> (std::string& value);
      // </group>
      
      // Get an array of values with the given number of values.
      // Bools are retrieved as bits.
      // <group>
      void get (bool* values, uint64 nrval);
      void get (char* values, uint64 nrval);
      void get (int8* values, uint64 nrval);
      void get (uint8* values, uint64 nrval);
      void get (int16* values, uint64 nrval);
      void get (uint16* values, uint64 nrval);
      void get (int32* values, uint64 nrval);
      void get (uint32* values, uint64 nrval);
      void get (int64* values, uint64 nrval);
      void get (uint64* values, uint64 nrval);
      void get (float* values, uint64 nrval);
      void get (double* values, uint64 nrval);
      template<class T> void get (std::complex<T>* values, uint64 nrval);
      void get (i4complex* values, uint64 nrval);
      void get (i16complex* values, uint64 nrval);
      void get (u16complex* values, uint64 nrval);
      void get (fcomplex* values, uint64 nrval);
      void get (dcomplex* values, uint64 nrval);
      void get (std::string* values, uint64 nrval);
      // </group>
      
      // Get a vector of values. First it gets the size of the vector.
      // Specialise for bool because a vector of bools uses bits.
      // <group>
      void get (std::vector<bool>& values);
      template<typename T> void get (std::vector<T>& values);
      // </group>
      
      // Get a vector of bools of 'size' elements.
      // The vector is resized to the given size.
      void getBoolVec (std::vector<bool>& values, uint64 size);
      
      // Skip the given amount of space (the opposite of BlobOStream::setSpace).
      // This is useful when reading a static blob in a dynamic way.
      // It returns the position of the skipped space in the stream.
      // It is meant for use with the BlobIBufString buffer. The function
      // getPointer in that class (in fact in its base class BlobIBufChar)
      // can be used to turn the position into a pointer.
      int64 getSpace (uint64 nbytes);
      
      // Skip as many filler bytes as needed to make total length a multiple of n.
      // In this way the next data are aligned.
      // It returns the number of filler bytes used.
      // It is only useful for seekable buffers.
      uint align (uint n);
      
      // Get the current stream position.
      // It returns -1 if the stream is not seekable.
      int64 tellPos() const;
      
      // Tell if conversion is needed and return the format of the data.
      // This can be done after a getStart.
      // <group>
      bool mustConvert() const;
      LOFAR::DataFormat dataFormat() const;
      // </group>
      
    private:
      // Read the buffer, increment itsCurLength, and check if everything read.
      void getBuf (void* buf, uint64 sz);
      
      // Throw an exception if a get cannot be done.
      // <group>
      void checkGet() const;
      void throwGet() const;
      // </group>
      
      
      bool   itsSeekable;
      bool   itsMustConvert;
      bool   itsHasCachedType;
      uint64 itsCurLength;
      uint   itsLevel;
      int    itsVersion;
      // The endian type of the data in the blob.
      LOFAR::DataFormat  itsDataFormat;
      // The cached object type.
      std::string        itsObjectType;
      // Object length to read at each level
      std::stack<uint64> itsObjTLN;
      // Object length at each level
      std::stack<uint64> itsObjLen;
      // The underlying stream object.
      BlobIBuffer*       itsStream;
    };
  
// </group>

  
  inline void BlobIStream::clear()
    {
      itsCurLength = 0;
      itsLevel     = 0;
      itsHasCachedType = false;
    }
  
  inline int BlobIStream::getStart (const char* objectType)
    { return getStart (std::string(objectType)); }
  
  inline int64 BlobIStream::tellPos() const
    { return itsStream->tellPos(); }
  
  inline bool BlobIStream::mustConvert() const
    { return itsMustConvert; }
  
  inline LOFAR::DataFormat BlobIStream::dataFormat() const
    { return itsDataFormat; }
  
  template<typename T>
    inline BlobIStream& BlobIStream::operator>> (std::complex<T>& value)
    {
      getBuf (&value, sizeof(std::complex<T>));
      if (itsMustConvert) {
	LOFAR::dataConvert (itsDataFormat, &value, 1);
      }
      return *this;
    }
  template<typename T>
    inline void BlobIStream::get (std::complex<T>* values, uint64 nrval)
    {
      getBuf (values, nrval*sizeof(std::complex<T>));
      if (itsMustConvert) {
	LOFAR::dataConvert (itsDataFormat, values, nrval);
      }
    }

  template<typename T>
    inline void BlobIStream::get (std::vector<T>& vec)
    {
      uint64 sz;
      operator>> (sz);
      vec.resize (sz);
      get (&vec[0], sz);
    }
  inline void BlobIStream::get (std::vector<bool>& vec)
    {
      uint64 sz;
      operator>> (sz);
      getBoolVec (vec, sz);
    }
  
  inline void BlobIStream::checkGet() const
    {
      if (itsLevel == 0) throwGet();
    }
  
} // end namespace

#endif
