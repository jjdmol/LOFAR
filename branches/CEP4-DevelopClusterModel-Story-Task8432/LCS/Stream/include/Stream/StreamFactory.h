//# StreamFactory.h: class/functions to construct streams
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef LOFAR_STREAM_STREAMFACTORY_H
#define LOFAR_STREAM_STREAMFACTORY_H

#include <ctime>
#include <string>
#include <Stream/Stream.h>

namespace LOFAR
{

  // Create a stream from a descriptor.
  // Caller should wrap the returned pointer in some smart ptr type. 
  //
  // deadline: absolute deadline for creating the connection
  Stream *createStream(const std::string &descriptor, bool asReader, time_t deadline = 0);

} // namespace LOFAR

#endif

