//# BlobIBufString.h: Input buffer for a blob using a string
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

#ifndef LOFAR_BLOB_BLOBIBUFSTRING_H
#define LOFAR_BLOB_BLOBIBUFSTRING_H

// \file
// Input buffer for a blob using a string

#include <Blob/BlobIBufChar.h>
#include <Blob/BlobString.h>
#include <string>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // The buffer can be static or dynamic. A static buffer has a fixed
  // length and cannot grow. A dynamic buffer can grow as needed.
  //
  // The class is meant to be used as BlobIBuffer<uchar> to be able to
  // read a blob from a string<uchar> for use in the PL classes and DTL.
  
  class BlobIBufString : public BlobIBufChar
    {
    public:
      // Construct from a string.
      explicit BlobIBufString (const BlobString& buffer)
	: BlobIBufChar (buffer.data(), buffer.size())
	{}
      
      // Destructor.
      virtual ~BlobIBufString()
	{}
    };

  // @}

} // end namespace

#endif
