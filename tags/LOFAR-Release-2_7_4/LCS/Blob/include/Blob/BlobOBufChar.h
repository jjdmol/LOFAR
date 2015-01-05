//# BlobOBufChar.h: Output buffer for a blob using a plain pointer
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

#ifndef LOFAR_BLOB_BLOBOBUFCHAR_H
#define LOFAR_BLOB_BLOBOBUFCHAR_H

// \file
// Output buffer for a blob using a plain pointer

#include <Blob/BlobOBuffer.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // This class is the BlobOBuffer that makes use of a character buffer,
  // i.e. a buffer of signed or unsigned chars.
  // The buffer can be static or dynamic. A static buffer (expandSize=0)
  // has a fixed length and cannot grow. A dynamic buffer can grow as needed.
  // The class is mainly meant as the base class for BlobOBufString, but can
  // also be used in itself.
  
  class BlobOBufChar : public BlobOBuffer
    {
    public:
      // Construct a dynamic buffer with the given initial length.
      explicit BlobOBufChar (uint64 initialSize=65536, uint64 expandSize=32768);
      
      // Construct from the given buffer with the given size.
      // The arguments have the following meaning:
      // <ul>
      // <li> buffer is a predefined buffer. The pointer can be 0 if size==0.
      // <li> size is the initial size of the buffer.
      // <li> expandSize tells if the buffer can be extended if it is full.
      //      It renews the internal buffer pointer which can be retrieved
      //      using function getBuffer.
      //      If expandSize==0 the buffer is static and cannot grow.
      // <li> start indicates where to start in a predefined buffer.
      //      It has to be < size.
      //      It makes it possible to append to an existing buffer.
      // <li> takeOver indicates if the buffer pointer is taken over,
      //      thus that buffer gets deleted by this class.
      // </ul>
      BlobOBufChar (void* buffer, uint64 size, uint64 expandSize=0, uint64 start=0,
		    bool takeOver=false);
      
      // Destructor.
      virtual ~BlobOBufChar();
      
      // Clear the buffer. I.e., set size and position to 0.
      void clear();
      
      // Put the requested nr of bytes.
      virtual uint64 put (const void* buffer, uint64 nbytes);
      
      // Get the position in the buffer.
      virtual int64 tellPos() const;
      
      // Set the position in the buffer.
      virtual int64 setPos (int64 pos);
      
      // Get the buffer containing the data.
      // <br>The size of the data in the buffer can be obtained using the
      // size() function.
      const uchar* getBuffer() const;
      
      // Get the size of the data in the buffer.
      uint64 size() const;
      
      // Get the capacity of the buffer.
      uint64 capacity() const;
      
      // Get the expand size (0 = not expandable).
      uint64 expandSize() const;
      
      // Get a typed pointer to an area in the string.
      // It is meant to be used in combination with BlobOStream::setSpace.
      template<typename U> U* getPointer (uint64 position);
      
      // Reserve at least the given size.
      // An exception is thrown if the buffer needs to be expanded, but cannot.
      void reserve (uint64 newReservedSize);
      
      // Resize the buffer.
      // An exception is thrown if the buffer needs to be expanded, but cannot.
      void resize (uint64 newSize);
      
    protected:
      // Set the buffer pointer.
      void setBuffer (void* buffer);

    private:
      // Expand the buffer if new size > current size.
      bool resizeIfNeeded (uint64 newSize);
      
      // Try to expand the buffer to at least the given size.
      // It returns false if the buffer cannot be expanded.
      bool expand (uint64 minSize);
      
      // Expand the buffer to the given size.
      virtual void doExpand (uint64 newReservedSize, uint64 newSize);
      
      
      uchar* itsBuffer;
      uint64 itsSize;
      uint64 itsPos;
      uint64 itsReservedSize;
      uint64 itsExpandSize;
      bool   itsIsOwner;
    };
  
// @}
  
  inline const uchar* BlobOBufChar::getBuffer() const
    {
      return itsBuffer;
    }
  inline uint64 BlobOBufChar::size() const
    {
      return itsSize;
    }
  inline uint64 BlobOBufChar::capacity() const
    {
      return itsReservedSize;
    }
  inline uint64 BlobOBufChar::expandSize() const
    {
      return itsExpandSize;
    }
  
  inline bool BlobOBufChar::resizeIfNeeded (uint64 newSize)
    {
      return (newSize > itsSize  ?  expand(newSize) : true);
    }
  inline void BlobOBufChar::setBuffer (void* buffer)
    {
      itsBuffer = static_cast<uchar*>(buffer);
    }
  
  template<typename U>
    inline U* BlobOBufChar::getPointer (uint64 position)
    {
      DBGASSERT(position < itsSize);
      return (U*)(itsBuffer + position);
    }
  

} // end namespace

#endif
