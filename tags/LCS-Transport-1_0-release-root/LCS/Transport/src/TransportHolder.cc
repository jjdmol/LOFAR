//# TransportHolder.cc:
//#
//# Copyright (C) 2000, 2001
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


#include <Transport/TransportHolder.h>
#include <Transport/DataHolder.h>
#include <Common/BlobStringType.h>
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

bool TransportHolder::recvBlocking (void*, int, int)
{
  THROW(LOFAR::Exception, "No recvBlocking method implemented in this TransportHolder: " 
	+ getType());
}

bool TransportHolder::recvVarBlocking (int /*tag*/)
{
  THROW(LOFAR::Exception, "No recvVarBlocking method implemented in this TransportHolder: " 
	+ getType());
}

bool TransportHolder::recvNonBlocking (void*, int, int)
{
  THROW(LOFAR::Exception, "No recvNonBlocking receive method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::recvVarNonBlocking (int /*tag*/)
{
  THROW(LOFAR::Exception, "No recvVarNonBlocking method implemented in this TransportHolder: " 
	+ getType());
}

bool TransportHolder::waitForReceived(void*, int, int)
{
  THROW(LOFAR::Exception, "No waitForReceived method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::sendBlocking (void*, int, int)
{
  THROW(LOFAR::Exception, "No sendBlocking method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::sendVarBlocking (void* buf, int nbytes, int tag)
{
  return sendBlocking (buf, nbytes, tag);
}

bool TransportHolder::sendNonBlocking (void*, int, int)
{
  THROW(LOFAR::Exception, "No sendNonBlocking method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::sendVarNonBlocking (void* buf, int nbytes, int tag)
{
  return sendNonBlocking (buf, nbytes, tag);
}

bool TransportHolder::waitForSent(void*, int, int)
{
  THROW(LOFAR::Exception, "No waitForSent method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::waitForRecvAck(void*, int, int)
{
  THROW(LOFAR::Exception, "No waitForRecvAck method implemented in this TransportHolder: "
	+ getType());
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

bool TransportHolder::connectionPossible (int, int) const
{
  return false;
}

bool TransportHolder::isBidirectional () const
{
  return false;
}

}
