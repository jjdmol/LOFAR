//# BlobOBufString.cc: Input buffer for a blob using a string
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobOBufString.h>

namespace LOFAR {

BlobOBufString::BlobOBufString (BlobString& buffer,
				uint expandSize, uint start)
  : BlobOBufChar (buffer.data(), buffer.capacity(),
		  buffer.canExpand() ? expandSize:0,
		  start, false),
    itsString    (&buffer)
{
  ASSERT(start <= buffer.size());
}

BlobOBufString::~BlobOBufString()
{}

void BlobOBufString::doExpand (uint newReservedSize, uint newSize)
{
  if (newReservedSize > itsString->capacity()) {
    itsString->reserve (newReservedSize);
  }
  itsString->resize (newSize);
  // Note that the std::string class might uses reference counting.
  // Therefore a resize may need to make a copy, so it can change the
  // buffer pointer. 
  setBuffer (itsString->data());
}

} // end namespace
