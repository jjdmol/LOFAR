//# BGLConnection.cc:
//#
//# Copyright (C) 2006
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

#include <lofar_config.h>

#include <Transport/BGLConnection.h>
#include <Transport/DataHolder.h>
#include <Transport/TransportHolder.h>
#include <Common/LofarLogger.h>
#include <stdlib.h>
#include <memory.h>


namespace LOFAR
{

BGLConnection::BGLConnection(const string& name, DataHolder* dhSource, DataHolder* dhDest, TransportHolder* th)
:
  Connection(name, dhSource, dhDest, th, true)
{
  LOG_TRACE_FLOW("BGLConnection constructor");
}


BGLConnection::~BGLConnection()
{
  LOG_TRACE_FLOW("BGLConnection destructor");
}


Connection::State BGLConnection::read()
{
  LOG_TRACE_FLOW("Connection read()");
  DBGASSERTSTR(itsDestDH != 0, "No destination DataHolder set!");
  DBGASSERTSTR(itsDestDH->isInitialized(), 
	       "Destination dataholder has not been initialized!");
  DBGASSERT(getTransportHolder() != 0);
  DBGASSERT(itsDestDH != 0);

  LOG_TRACE_COND_STR("Transport::read; call recv with tag " << getTag());

  if (itsDestDH->hasFixedSize()) {    
    if (!getTransportHolder()->recvBlocking(itsDestDH->getDataPtr(),
					    align(itsDestDH->getDataSize(), 16),
					    getTag(), 0, itsDestDH))
      return Error;
  } else {
    // Read blob header first (plus possibly some extra bytes)
    uint headerSize = align(itsDestDH->getHeaderSize(), 16);

    if (!getTransportHolder()->recvBlocking(itsDestDH->getDataPtr(), headerSize,
					    getTag(), 0, itsDestDH))
      return Error;

    unsigned totalLength = DataHolder::getDataLength(itsDestDH->getDataPtr());
    itsDestDH->resizeBuffer(totalLength);

    unsigned dataLength = align(totalLength - headerSize, 16);
    char     *dataPtr   = static_cast<char*>(itsDestDH->getDataPtr()) + headerSize;

    if (!getTransportHolder()->recvBlocking(dataPtr, dataLength,
					    getTag(), headerSize, itsDestDH))
      return Error;
  }

  itsDestDH->unpack();
  return Finished;
}


Connection::State BGLConnection::write()
{
  LOG_TRACE_FLOW("Connection write()");
  DBGASSERTSTR(itsSourceDH != 0, "No source DataHolder set!");
  DBGASSERTSTR(itsSourceDH->isInitialized(), 
	       "Source dataholder has not been initialized!");
  DBGASSERT(getTransportHolder() != 0);
  DBGASSERT(itsSourceDH != 0);

  LOG_TRACE_COND_STR("Transport::write; call send with tag " << getTag());

  itsSourceDH->pack();

  if (itsSourceDH->hasFixedSize()) {    
    if (!getTransportHolder()->sendBlocking(itsSourceDH->getDataPtr(),
					    align(itsSourceDH->getDataSize(), 16),
					    getTag(), itsSourceDH))
      return Error;
  } else {
    // Read and write sizes should match.  Send blob header first.
    unsigned headerSize = align(itsSourceDH->getHeaderSize(), 16);

    if (!getTransportHolder()->sendBlocking(itsSourceDH->getDataPtr(),
					    headerSize, getTag(),
					    itsSourceDH))
      return Error;

    unsigned totalLength = DataHolder::getDataLength(itsSourceDH->getDataPtr());
    unsigned dataLength	 = align(totalLength - headerSize, 16);
    char*    dataPtr	 = static_cast<char*>(itsSourceDH->getDataPtr()) + headerSize;

    if (!getTransportHolder()->sendBlocking(dataPtr, dataLength,
					    getTag(), itsSourceDH))
      return Error;
  }

  return Finished;
}


} // end namespace
