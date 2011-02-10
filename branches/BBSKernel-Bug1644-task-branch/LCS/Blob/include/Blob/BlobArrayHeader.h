//# BlobArrayHeader.h: Standard array header for a blob
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
      uchar  itsNAlign[5];    // number of filler bytes for proper alignment
      uint16 itsNdim;
      uint64 itsSize[NDIM];   // shape
    };

  // @}

} // end namespace

#endif
