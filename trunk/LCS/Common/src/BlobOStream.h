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

#ifndef COMMON_BLOBOSTREAM_H
#define COMMON_BLOBOSTREAM_H

#include <Common/LofarTypes.h>
#include <Common/BlobOBuffer.h>
#include <stack>
#include <vector>
#include <string>

// This class makes it possible to create a blob.
// It creates a header (in the putStart function) using the
// BlobHeader definition.
// The user can define overloaded operator<< functions to be able
// to put objects of a given class into the blob stream.
//
// The blob is written into a BlobOBuffer object that can be a memory
// buffer or an ostream object. The BlobIStream class can be used to
// retrieve objects from a blob.

class BlobOStream
{
public:
  // Construct it with the underlying buffer object.
  // If header8==true, the blob headers will be made a multiple of 8 bytes.
  explicit BlobOStream (BlobOBuffer*, bool header8 = false);

  // Destructor closes the stream if not closed yet.
  ~BlobOStream();

  // Get the total size.
  uint64 size() const;

  // Start putting a blob.
  // Data in the outermost object cannot be put before a putstart is done.
  // Data in nested objects can be put without an intermediate putstart.
  // However, for complex objects it is recommended to do a putstart
  // to have a better checking.
  // <br>
  // After all values (inclusing nested objects) of the object have
  // been put, a call to putend has to be done.
  // It returns the nesting level.
  // <group>
  uint putStart (const std::string& objectType, int objectVersion);
  uint putStart (const char* objectType, int objectVersion);
  // </group>

  // End putting an object. It returns the object length (including
  // possible nested objects).
  uint putEnd();

  // Put a single value.
  // A bool will be stored as a char.
  // A string will be stored as a length followed by the characters.
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
  BlobOStream& operator<< (const fcomplex& value);
  BlobOStream& operator<< (const dcomplex& value);
  BlobOStream& operator<< (const std::string& value);
  BlobOStream& operator<< (const char* value);
  // </group>

  // Put an array of values with the given number of values.
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
  void put (const fcomplex* values, uint nrval);
  void put (const dcomplex* values, uint nrval);
  void put (const std::string* values, uint nrval);
  // </group>

  // Put a vector of values.
  // Specialise for bool because a vector of bools uses bits.
  // <group>
  void put (const std::vector<bool>& values);
  template<typename T> void put (const std::vector<T>& values);
  // </group>

  // Reserve the given amount of space (the opposite of BlobIStream::getSpace).
  // This is useful when reading a static blob in a dynamic way.
  // It returns the position of the skipped space in the stream.
  // It is meant for use with the BlobOBufString buffer. The function
  // getPointer in that class can be used to turn the position into a pointer.
  int64 setSpace (uint nbytes);

  // Reserve the given number of bytes.
  // This is useful when forming a static blob in a dynamic way.
  // It returns the position of the reserved space in the stream.
  int64 reserve (uint nbytes);

  // Add filler bytes as needed to make total length a multiple of n.
  // In this way the next data are aligned.
  // It returns the number of filler bytes used.
  // It is only useful for seekable buffers.
  uint align (uint n);

  // Get the current stream position.
  int64 tellPos() const;

private:
  uint doPutStart (const char* objectType, uint nrc, int objectVersion);

  bool   itsHeader8;
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


inline uint BlobOStream::putStart (const std::string& objectType,
				   int objectVersion)
  { return doPutStart (objectType.data(), objectType.size(), objectVersion); }

inline uint BlobOStream::putStart (const char* objectType,
				   int objectVersion)
  { return doPutStart (objectType, strlen(objectType), objectVersion); }

inline int64 BlobOStream::tellPos() const
  { return itsStream->tellPos(); }

template<typename T>
inline void BlobOStream::put (const std::vector<T>& vec)
{
  operator<< (uint32(vec.size()));
  put (&vec[0], vec.size());
}


#endif
