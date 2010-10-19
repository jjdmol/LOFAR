//# BlobHeader.tcc: Standard header for a blob
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

#ifndef COMMON_BLOBHEADER_TCC
#define COMMON_BLOBHEADER_TCC

#include <Common/BlobHeader.h>
#include <Common/DataFormat.h>
#include <Common/Debug.h>


// Use the same magic value as used in the AIPS++ class AipsIO.
// It is insensitive to data format (big or little endian).

template<uint NAMELENGTH>
LOFAR::BlobHeader<NAMELENGTH>::BlobHeader (const char* objectType, int version,
					   uint level)
: itsMagicValue     (BlobHeaderBase::bobMagicValue()),
  itsLength         (0),
  itsVersion        (version),
  itsDataFormat     (LOFAR::dataFormat()),
  itsLevel          (level),
  itsReservedLength (sizeof(itsName))
{
  uint len = strlen(objectType);
  Assert (len <= NAMELENGTH);
  Assert (len < 256);
  memcpy (itsName, objectType, len);
  itsNameLength = len;
  DbgAssert (plainSize() == (char*)(&itsName) - (char*)this);
}
    
template<uint NAMELENGTH>
void LOFAR::BlobHeader<NAMELENGTH>::setLocalDataFormat()
{
  itsLength     = LOFAR::dataConvert (getDataFormat(), itsLength);
  itsVersion    = LOFAR::dataConvert (getDataFormat(), itsVersion);
  itsDataFormat = LOFAR::dataFormat();
}

#endif
