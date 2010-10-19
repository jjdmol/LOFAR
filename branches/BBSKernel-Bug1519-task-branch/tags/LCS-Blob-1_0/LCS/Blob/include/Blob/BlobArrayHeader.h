//# BlobArrayHeader.h: Standard array header for a blob
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

#ifndef LOFAR_BLOB_BLOBARRAYHEADER_H
#define LOFAR_BLOB_BLOBARRAYHEADER_H

// \file
// Standard array header for a blob

#include <Blob/BlobHeader.h>
#include <Common/TypeNames.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

// \ingroup %pkgname%
  // @{
  
  // An array is a special blob, so it has an array header following
  // the standard blob header as defined in BlobHeader.
  // <br>This class defines the standard array header.
  // The class is not really used, but tells what the array header used in
  // BlobArray is like.
  
  template<class NDIM>
    class BlobArrayHeader
    {
    private:
      char   itsAxisOrder;
      uchar  itsNAlign;       // number of filler bytes for proper alignment
      uint16 itsNdim;
      uint32 itsSize[NDIM];   // shape
    };

  // @}

} // end namespace

#endif
