//# BlobIBufVector.h: Input buffer for a blob using a vector
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
