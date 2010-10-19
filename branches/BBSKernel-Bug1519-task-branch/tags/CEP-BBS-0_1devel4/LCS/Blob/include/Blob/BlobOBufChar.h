//# BlobOBufChar.h: Output buffer for a blob using a plain pointer
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
      explicit BlobOBufChar (uint initialSize=65536, uint expandSize=32768);
      
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
      BlobOBufChar (void* buffer, uint size, uint expandSize=0, uint start=0,
		    bool takeOver=false);
      
      // Destructor.
      virtual ~BlobOBufChar();
      
      // Clear the buffer. I.e., set size and position to 0.
      void clear();
      
      // Put the requested nr of bytes.
      virtual uint put (const void* buffer, uint nbytes);
      
      // Get the position in the buffer.
      virtual int64 tellPos() const;
      
      // Set the position in the buffer.
      virtual int64 setPos (int64 pos);
      
      // Get the buffer containing the data.
      // <br>The size of the data in the buffer can be obtained using the
      // size() function.
      const uchar* getBuffer() const;
      
      // Get the size of the data in the buffer.
      uint size() const;
      
      // Get the capacity of the buffer.
      uint capacity() const;
      
      // Get the expand size (0 = not expandable).
      uint expandSize() const;
      
      // Get a typed pointer to an area in the string.
      // It is meant to be used in combination with BlobOStream::setSpace.
      template<typename U> U* getPointer (uint position);
      
      // Reserve at least the given size.
      // An exception is thrown if the buffer needs to be expanded, but cannot.
      void reserve (uint newReservedSize);
      
      // Resize the buffer.
      // An exception is thrown if the buffer needs to be expanded, but cannot.
      void resize (uint newSize);
      
    protected:
      // Set the buffer pointer.
      void setBuffer (void* buffer);

    private:
      // Expand the buffer if new size > current size.
      bool resizeIfNeeded (uint newSize);
      
      // Try to expand the buffer to at least the given size.
      // It returns false if the buffer cannot be expanded.
      bool expand (uint minSize);
      
      // Expand the buffer to the given size.
      virtual void doExpand (uint newReservedSize, uint newSize);
      
      
      uchar* itsBuffer;
      uint   itsSize;
      uint   itsPos;
      uint   itsReservedSize;
      uint   itsExpandSize;
      bool   itsIsOwner;
    };
  
// @}
  
  inline const uchar* BlobOBufChar::getBuffer() const
    {
      return itsBuffer;
    }
  inline uint BlobOBufChar::size() const
    {
      return itsSize;
    }
  inline uint BlobOBufChar::capacity() const
    {
      return itsReservedSize;
    }
  inline uint BlobOBufChar::expandSize() const
    {
      return itsExpandSize;
    }
  
  inline bool BlobOBufChar::resizeIfNeeded (uint newSize)
    {
      return (newSize > itsSize  ?  expand(newSize) : true);
    }
  inline void BlobOBufChar::setBuffer (void* buffer)
    {
      itsBuffer = static_cast<uchar*>(buffer);
    }
  
  template<typename U>
    inline U* BlobOBufChar::getPointer (uint position)
    {
      DBGASSERT(position < itsSize);
      return (U*)(itsBuffer + position);
    }
  

} // end namespace

#endif
