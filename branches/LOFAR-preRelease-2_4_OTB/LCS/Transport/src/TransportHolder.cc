//# TransportHolder.cc:
//#
//# Copyright (C) 2000, 2001
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transport/TransportHolder.h>
#include <Transport/DataHolder.h>
#include <Blob/BlobStringType.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

TransportHolder::TransportHolder()
{
  LOG_TRACE_FLOW("TransportHolder constructor");
}

TransportHolder::~TransportHolder()
{
  LOG_TRACE_FLOW("TransportHolder destructor");
}

bool TransportHolder::init()
{
  LOG_TRACE_RTTI("TransportHolder init()");
  return true;
}

void TransportHolder::readTotalMsgLengthBlocking(int, int& nrBytes)
{
  nrBytes = -1;
}

bool TransportHolder::readTotalMsgLengthNonBlocking(int, int& nrBytes)
{
  nrBytes = -1;
  return true;
}

bool TransportHolder::isConnected () const
{
  return true;
}

BlobStringType TransportHolder::blobStringType() const
{
  // Use a char* buffer on the heap.
  return BlobStringType(false);
}

bool TransportHolder::canDataGrow() const
{
  return true;
}

TransportHolder* TransportHolder::clone() const
{
  THROW(LOFAR::Exception, "TransportHolder " + getType() 
	+ " is not clonable.");
}

}
