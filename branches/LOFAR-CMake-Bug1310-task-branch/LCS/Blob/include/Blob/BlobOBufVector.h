//# BlobOBufVector.h: Input buffer for a blob using a vector
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

#ifndef LOFAR_BLOB_BLOBOBUFVECTOR_H
#define LOFAR_BLOB_BLOBOBUFVECTOR_H

// \file
// Input buffer for a blob using a vector

#include <Blob/BlobOBufChar.h>
#include <Common/LofarLogger.h>
#include <vector>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // This class is the BlobOBuffer that makes use of a vector object.
  // The vector can be static or dynamic. A static vector has a fixed
  // length and cannot grow. A dynamic vector can grow as needed.
  //
  // The class is templated. However, the template parameter can only be
  // a char or unsigned char. The constructor checks if sizeof(T)==1.
  
  template<typename T>
    class BlobOBufVector : public BlobOBufChar
    {
    public:
      // Construct from a buffer with the given vector.
      // If expandSize==0, the vector is static and cannot grow.
      // The argument start can be used to append to an existing string.
      // It keeps a pointer to the given vector object, so that should
      // not be deleted before this object.
      explicit BlobOBufVector (std::vector<T>& buffer,
			       uint expandSize=1024, uint start=0)
	: BlobOBufChar(buffer.empty()  ?  0 : &(buffer[0]),
		       buffer.capacity(), expandSize, start, false),
	  itsVector   (&buffer)
	{
	  ASSERT(sizeof(T)==1);
          ASSERT(start <= buffer.size());
	}
      
      // Destructor.
      virtual ~BlobOBufVector()
	{}
      
    private:
      // Expand the capacity of the buffer to the given size.
      virtual void doExpand (uint newReservedSize, uint newSize)
	{
	  if (newReservedSize > itsVector->capacity()) {
	    itsVector->reserve (newReservedSize);
	  }
	  itsVector->resize (newSize);
	  if (newSize > 0) {
	    setBuffer (&((*itsVector)[0]));
	  }
	}
      
      std::vector<T>* itsVector;
    };

  // @}

} // end namespace

#endif
