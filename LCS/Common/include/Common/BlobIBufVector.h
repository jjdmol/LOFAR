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

#ifndef COMMON_BLOBIBUFVECTOR_H
#define COMMON_BLOBIBUFVECTOR_H

#include <Common/BlobIBufChar.h>
#include <vector>


// The buffer can be static or dynamic. A static buffer has a fixed
// length and cannot grow. A dynamic buffer can grow as needed.
//
// The class is meant to be used as BlobIBuffer<uchar> to be able to
// read a blob from a vector<uchar> for use in the PL classes and DTL.

template<typename T>
class BlobIBufVector : public BlobIBufChar
{
public:
  // Construct from a vector.
  explicit BlobIBufVector (const std::vector<T>& buffer)
    : BlobIBufChar(&buffer[0], buffer.size()*sizeof(T))
    {}

  // Destructor.
  virtual ~BlobIBufVector()
    {}
};


#endif
