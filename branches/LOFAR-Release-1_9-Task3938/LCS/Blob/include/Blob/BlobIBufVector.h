//# BlobIBufVector.h: Input buffer for a blob using a vector
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

#ifndef LOFAR_BLOB_BLOBIBUFVECTOR_H
#define LOFAR_BLOB_BLOBIBUFVECTOR_H

// \file
// Input buffer for a blob using a vector

#include <Blob/BlobIBufChar.h>
#include <vector>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // This class can be used to read a blob from a vector of characters
  // (signed or unsigned).
  
  template<typename T>
    class BlobIBufVector : public BlobIBufChar
    {
    public:
      // Construct from a vector.
      explicit BlobIBufVector (const std::vector<T>& buffer)
	: BlobIBufChar(buffer.empty()  ?  0 : &(buffer[0]),
		       buffer.size()*sizeof(T))
      {}
      
      // Destructor.
      virtual ~BlobIBufVector()
	{}
    };

  // @}

} // end namespace

#endif
