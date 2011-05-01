//# NullStream.cc: 
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>

#include <Stream/NullStream.h>
#include <Thread/Cancellation.h>

#include <cstring>


namespace LOFAR {

NullStream::~NullStream()
{
}


size_t NullStream::tryRead(void *ptr, size_t size)
{
  Cancellation::point(); // keep behaviour consistent with non-null streams

  memset(ptr, 0, size);
  return size;
}


size_t NullStream::tryWrite(const void *, size_t size)
{
  Cancellation::point(); // keep behaviour consistent with non-null streams

  return size;
}

} // namespace LOFAR
