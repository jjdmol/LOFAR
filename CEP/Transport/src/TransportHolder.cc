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
#include <Common/BlobStringType.h>
#include <Common/Debug.h>

namespace LOFAR
{

TransportHolder::TransportHolder()
{}

TransportHolder::~TransportHolder()
{}

bool TransportHolder::init()
{
  return true;
}

bool TransportHolder::recvBlocking (void*, int, int)
{
  Throw("No blocking receive method implemented in this TransportHolder: " 
	+ getType());
}

bool TransportHolder::recvNonBlocking (void*, int, int)
{
  Throw("No non-blocking receive method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::waitForReceived(void*, int, int)
{
  Throw("No waitForReceived() method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::sendBlocking (void*, int, int)
{
  Throw("No blocking send method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::sendNonBlocking (void*, int, int)
{
  Throw("No non-blocking send method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::waitForSent(void*, int, int)
{
  Throw("No waitForSent() method implemented in this TransportHolder: "
	+ getType());
}

bool TransportHolder::waitForRecvAck(void*, int, int)
{
  Throw("No waitForRecvAck() method implemented in this TransportHolder: "
	+ getType());
}

BlobStringType TransportHolder::blobStringType() const
{
  // Use a char* buffer on the heap.
  return BlobStringType(false);
}

bool TransportHolder::connectionPossible (int, int) const
{
  return false;
}

}
