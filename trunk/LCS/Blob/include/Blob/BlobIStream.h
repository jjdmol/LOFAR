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

#ifndef COMMON_BLOBISTREAM_H
#define COMMON_BLOBISTREAM_H

#include <Common/LofarTypes.h>
#include <Common/DataFormat.h>
#include <Common/BlobIBuffer.h>
#include <stack>
#include <vector>
#include <string>

namespace LOFAR {

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

class BlobIStream
{
public:
  // Construct it with the underlying buffer object.
  explicit BlobIStream (BlobIBuffer*);

  // Destructor closes the stream if not closed yet.
  ~BlobIStream();

  // Start getting a blob.
  // Data in the outermost object cannot be get before a getstart is done.
  // Data in nested objects can be get without an intermediate getstart.
  // However, for complex objects it is recommended to do a getstart
  // to have a better checking.
  // <br>
  // After all values (inclusing nested objects) of the object have
  // been get, a call to getend has to be done.
  // It returns the nesting level.
  // <group>
  int getStart (const std::string& objectType);
  int getStart (const char* objectType);
  // </group>

  // End getting an object. It returns the object length (including
  // possible nested objects).
  // It checks if the entire object has been read (to keep the data
  // stream in sync). If not, an exception is thrown.
  uint getEnd();

  // getNextType gets the object type of the next piece of
  // information to read.
  // It checks if it finds the correct magic value preceeding
  // the object type.
  const std::string& BlobIStream::getNextType();

  // Get a single value.
  // A bool will be stored as a char.
  // A string will be stored as a length followed by the characters.
  // <group>
  BlobIStream& operator>> (bool& value);
  BlobIStream& operator>> (char& value);
  BlobIStream& operator>> (uchar& value);
  BlobIStream& operator>> (int16& value);
  BlobIStream& operator>> (uint16& value);
  BlobIStream& operator>> (int32& value);
  BlobIStream& operator>> (uint32& value);
  BlobIStream& operator>> (int64& value);
  BlobIStream& operator>> (uint64& value);
  BlobIStream& operator>> (float& value);
  BlobIStream& operator>> (double& value);
  BlobIStream& operator>> (fcomplex& value);
  BlobIStream& operator>> (dcomplex& value);
  BlobIStream& operator>> (std::string& value);
  // </group>

  // Get an array of values with the given number of values.
  // Bools will be compressed to single bits.
  // <group>
  void get (bool* values, uint nrval);
  void get (char* values, uint nrval);
  void get (uchar* values, uint nrval);
  void get (int16* values, uint nrval);
  void get (uint16* values, uint nrval);
  void get (int32* values, uint nrval);
  void get (uint32* values, uint nrval);
  void get (int64* values, uint nrval);
  void get (uint64* values, uint nrval);
  void get (float* values, uint nrval);
  void get (double* values, uint nrval);
  void get (fcomplex* values, uint nrval);
  void get (dcomplex* values, uint nrval);
  void get (std::string* values, uint nrval);
  // </group>

  // Get a vector of values.
  // Specialise for bool because a vector of bools uses bits.
  // <group>
  void get (std::vector<bool>& values);
  template<typename T> void get (std::vector<T>& values);
  // </group>

  // Skip the given amount of space (the opposite of BlobOStream::setSpace).
  // This is useful when reading a static blob in a dynamic way.
  // It returns the position of the skipped space in the stream.
  // It is meant for use with the BlobIBufString buffer. The function
  // getPointer in that class can be used to turn the position into a pointer.
  int64 getSpace (uint nbytes);

  // Read filler bytes as needed to make total length a multiple of n.
  // In this way the next data are aligned.
  // It returns the number of filler bytes used.
  // It is only useful for seekable buffers.
  uint align (uint n);

  // Get the current stream position.
  int64 tellPos() const;

private:
  // Read the buffer, increment itsCurLength, and check if everything read.
  void getBuf (void* buf, uint sz);


  bool   itsSeekable;
  bool   itsMustConvert;
  bool   itsHasCachedType;
  uint32 itsCurLength;
  uint   itsLevel;
  int    itsVersion;
  // The endian type of the data in the blob.
  LOFAR::DataFormat  itsDataFormat;
  // The cached object type.
  std::string        itsObjectType;
  // Object length to read at each level
  std::stack<uint32> itsObjTLN;
  // Object length at each level
  std::stack<uint32> itsObjLen;
  // The underlying stream object.
  BlobIBuffer*       itsStream;
};


inline int BlobIStream::getStart (const char* objectType)
  { return getStart (std::string(objectType)); }

inline int64 BlobIStream::tellPos() const
  { return itsStream->tellPos(); }

template<typename T>
inline void BlobIStream::get (std::vector<T>& vec)
{
  uint32 sz;
  operator>> (sz);
  vec.resize (sz);
  get (&vec[0], sz);
}

} // end namespace

#endif
