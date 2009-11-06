//# BlobString.h: Blob buffer that can be a string<uchar> or char
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

#ifndef LOFAR_BLOB_BLOBSTRING_H
#define LOFAR_BLOB_BLOBSTRING_H

// \file
// Blob buffer that can be a string<uchar> or char

#include <Common/LofarTypes.h>
#include <Blob/BlobStringType.h>
#include <Blob/BlobStringTraits.h>
#include <string>

namespace LOFAR {

// \ingroup %pkgname%
// <group>

  // A BlobString object can hold a blob (binary large object).
  // Blobs are created by the BlobOStream class and read back by BlobIStream.
  // An BlobOBufString object has to be used to make a BlobString object
  // available for BlobOStream.
  // The main purpose of BlobString is the hide the way of allocating memory
  // for the blob. A BlobStringType object defines the way of allocation.
  //
  // The main goal is the transparent use of blobs in the TransportHolder
  // objects in CEPFrame.
  // <ol>
  //  <li> It can be a char* buffer in shared memory for TH_ShMem.
  //  <li> It can be a char* buffer on the heap for, say, TH_Mem or TH_MPI.
  //  <li> It can be a string<uchar> for TH_PL.
  // </ol>
  // Normally the buffer will expand as needed. However, it is possible to
  // tell that the buffer has a fixed size. In such a case an exception is
  // thrown if the buffer is too small.
  // Please note that a BlobOBufNull object can be used to find the exact
  // length of a blob before allocating the BlobString.
  
  class BlobString
    {
    public:
      // Create a buffer as a char* or string<uchar> (string is default).
      // By default the initial capacity can be extended.
      // The alignment argument tells if a char* buffer needs to be
      // aligned specifically. It is not used for a string<uchar> buffer.
      // The alignment has to be a power of 2.
      explicit BlobString (bool useString=true, size_t capacity=0,
			   bool canIncreaseCapacity=true,
			   uint alignment=0);
      
      // Create a data buffer with the given capacity.
      // The buffer is a char* or a string<uchar> depending on the allocator
      // object.
      // By default the initial capacity can be extended.
      // The alignment argument tells if a char* buffer needs to be
      // aligned specifically. It is not used for a string<uchar> buffer.
      // The alignment has to be a power of 2.
      explicit BlobString (const BlobStringType&, size_t capacity=0,
			   bool canIncreaseCapacity=true,
			   uint alignment=0);
      
      ~BlobString();
      
      // Get the size.
      size_t size() const
	{ return itsSize; }
      
      // Resize the data buffer. Nothing is done if the new size is not more.
      // If needed, the capacity is increased as well.
      void resize (size_t newSize);
      
      // Get the capacity.
      size_t capacity() const
	{ return itsCapacity; }
      
      // Can the capacity be extended?
      bool canExpand() const
	{ return itsCanIncr; }
      
      // Set the capacity. Nothing is done if the new capacity is not more.
      // An exception is thrown if the capacity has to be increased, but was
      // denied in the constructor.
      void reserve (size_t newSize);
      
      // Get a pointer to the data.
      // <group>
      const char* data() const
	{ return (const char*)itsBuffer; }
      char* data()
	{ return (char*)itsBuffer; }
      // </group>
      
      // Get the data as a string of uchar.
      // It throws an exception if not allocated that way.
      // <group>
      std::basic_string<uchar>& getString();
      const std::basic_string<uchar>& getString() const;
      // </group>
      
    private:
      // Forbid copy constructor and assignment.
      // <group>
      BlobString (const BlobString&);
      BlobString& operator= (const BlobString&);
      // </group>
      
      
      BlobStringType           itsAllocator;
      size_t                   itsCapacity;
      size_t                   itsSize;
      bool                     itsCanIncr;
      uint                     itsAlignment;
      void*                    itsBuffer;     // pointer to aligned buffer
      std::basic_string<uchar> itsString;
    };
  
// </group>
  
  inline const std::basic_string<uchar>& BlobString::getString() const
    {
      return const_cast<BlobString*>(this)->getString();
    }
  
} // end namespace

#endif
