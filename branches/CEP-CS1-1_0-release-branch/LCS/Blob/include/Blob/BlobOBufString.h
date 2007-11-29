//# BlobOBufString.h: Input buffer for a blob using a string
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

#ifndef LOFAR_BLOB_BLOBOBUFSTRING_H
#define LOFAR_BLOB_BLOBOBUFSTRING_H

// \file
// Input buffer for a blob using a string

#include <Blob/BlobOBufChar.h>
#include <Blob/BlobString.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{

  // This class is the BlobOBuffer that makes use of a BlobString object.
  // The string can be static or dynamic. A static string has a fixed
  // length and cannot grow. A dynamic string can grow as needed.
  
  class BlobOBufString : public BlobOBufChar
    {
    public:
      // Construct from a buffer with the given string.
      // If expandSize==0, the string is static and cannot grow.
      // The argument start can be used to append to an existing string.
      // It keeps a pointer to the given BlobString object, so that should
      // not be deleted before this object.
      explicit BlobOBufString (BlobString& buffer,
			       uint expandSize=1024, uint start=0);
      
      // Destructor.
      virtual ~BlobOBufString();
      
    private:
      // Expand the capacity of the buffer to the given size.
      virtual void doExpand (uint newReservedSize, uint newSize);
      
      
      BlobString* itsString;
    };

  // @}

} // end namespace

#endif
