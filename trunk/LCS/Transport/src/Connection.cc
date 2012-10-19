//# Connection.cc:
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

#include <lofar_config.h>

#include <Transport/Connection.h>
#include <Transport/DataHolder.h>
#include <Transport/TransportHolder.h>
#include <Common/LofarLogger.h>
#include <stdlib.h>
#include <memory.h>


namespace LOFAR
{
int Connection::theirNextTag = 0;

Connection::Connection (const string& name, DataHolder* dhSource, DataHolder* dhDest,
			TransportHolder* th, bool blockingComm)
  : itsSourceDH        (dhSource),
    itsDestDH          (dhDest),
    itsName            (name), 
    itsTransportHolder (th),
    itsIsBlocking      (blockingComm),
    itsReadState       (Idle),
    itsLastReadPtr     (0),
    itsLastReadSize    (0),
    itsWriteState      (Finished)
{
  LOG_TRACE_FLOW("Connection constructor");

  if (itsSourceDH && itsDestDH) {
    ASSERTSTR(dhSource->getType() == dhDest->getType(), 
	    "Connected DataHolders should be of the same type.");
    ASSERTSTR(dhSource->getVersion() == dhDest->getVersion(),
	    "Connected DataHolders should have the same version number");
  }

  itsTag = theirNextTag++;

  if (itsSourceDH && !itsSourceDH->isInitialized()) {
	itsSourceDH->setAllocationProperties(th->canDataGrow(), 
					     th->blobStringType());
  }
  if (itsDestDH && !itsDestDH->isInitialized()) {
	itsDestDH->setAllocationProperties(th->canDataGrow(), 
					   th->blobStringType());
  }
}

Connection::~Connection()
{
  LOG_TRACE_FLOW("Connection destructor");
}

Connection::State Connection::read()
{
  LOG_TRACE_FLOW("Connection read()");
  DBGASSERTSTR(itsDestDH != 0, "No destination DataHolder set!");
  DBGASSERTSTR(itsDestDH->isInitialized(), 
	       "Destination dataholder has not been initialized!");
  bool result = false;

  DBGASSERT(getTransportHolder() != 0);
  DBGASSERT(itsDestDH != 0);

  LOG_TRACE_COND_STR("Transport::read; call recv with tag " << getTag());

  if (isBlocking()) 
  {
    result = readBlocking();
    if (result)
    {
      itsDestDH->unpack();
      return Finished;
    }
    else
    {
      return Error;
    }
  }
  else
  {
    result = readNonBlocking();
    if (result)
    {
      itsDestDH->unpack();
      return Finished;
    }
    else
    {
      return Busy;
    }
  }
}

bool Connection::readBlocking()
{
  bool result = false;

  LOG_TRACE_COND("Connection:readBlocking");

  if (itsDestDH->hasFixedSize())
  {    
    result = getTransportHolder()->recvBlocking(itsDestDH->getDataPtr(),
						itsDestDH->getDataSize(),
						getTag(), 0, itsDestDH);
  }
  else
  {
    int msgLength = -1;
    getTransportHolder()->readTotalMsgLengthBlocking(getTag(), msgLength);
    if (msgLength > 0)
    {
      itsDestDH->resizeBuffer(msgLength);
      result = getTransportHolder()->recvBlocking(itsDestDH->getDataPtr(),
						  msgLength, getTag(), 
						  0, itsDestDH);
    }
    else
    {
      // Read blob header first
      uint headerSize = itsDestDH->getHeaderSize();
      result = getTransportHolder()->recvBlocking(itsDestDH->getDataPtr(),
						  headerSize, getTag(),
						  0, itsDestDH);
      if (result)
      {
	uint dataLength = DataHolder::getDataLength(itsDestDH->getDataPtr()) - headerSize;
	itsDestDH->resizeBuffer(dataLength+headerSize);

	char* dataPtr = static_cast<char*>(itsDestDH->getDataPtr()) + headerSize;
	result = getTransportHolder()->recvBlocking(dataPtr, dataLength,
						    getTag(), headerSize, itsDestDH);

      }
    }
  }
  return result;
}

bool Connection::readNonBlocking()
{
  LOG_TRACE_COND("Connection:readNonBlocking");

  if (itsReadState != Idle)
  {
    LOG_TRACE_COND("Previous nonblocking read has not yet finished. Waiting for this...");
    waitForRead();
  }
 
  bool result;

  if (itsDestDH->hasFixedSize())
  {
    result = getTransportHolder()->recvNonBlocking(itsDestDH->getDataPtr(),
						   itsDestDH->getDataSize(),
						   getTag(), 0, itsDestDH);
    if (!result)
    {
      itsReadState = Message;                  // Still busy reading message
      itsLastReadPtr = itsDestDH->getDataPtr();
      itsLastReadSize = itsDestDH->getDataSize();
    }
    return result;
  }
  else     // Variable size data
  {
    // Try reading the total message length
    int msgLength;
    result = getTransportHolder()->readTotalMsgLengthNonBlocking(getTag(), msgLength);
    if (!result)
    {
      itsReadState = TotalLength;              // Still busy reading total length
      return false;
    }
    else
    {
      if (msgLength > 0)
      {
	itsDestDH->resizeBuffer(msgLength);
	// Read total message
	result = getTransportHolder()->recvNonBlocking(itsDestDH->getDataPtr(),
						       msgLength, getTag(),
						       0, itsDestDH);
	if (!result)
	{
	  itsReadState = Message;              // Still busy reading message
	  itsLastReadPtr = itsDestDH->getDataPtr();
	  itsLastReadSize = msgLength;
	}
	return result;
      }
      else
      {
	// Read blob header first
	uint headerSize = itsDestDH->getHeaderSize();
	result = getTransportHolder()->recvNonBlocking(itsDestDH->getDataPtr(),
						       headerSize, getTag(),
						       0, itsDestDH);

	if (!result)
	{
	  itsReadState = Header;               // Still busy reading header
	  itsLastReadPtr = itsDestDH->getDataPtr();
	  itsLastReadSize = headerSize;
	  return false;
	}

	// Read rest of message
	uint dataLength = DataHolder::getDataLength(itsDestDH->getDataPtr()) - headerSize;
	itsDestDH->resizeBuffer(dataLength+headerSize);
	char* dataPtr = static_cast<char*>(itsDestDH->getDataPtr()) + headerSize;

	result = getTransportHolder()->recvNonBlocking(dataPtr, dataLength,
						       getTag(), headerSize, itsDestDH);

	if (!result)
	{
	  itsReadState = Message;             // Still busy reading message
	  itsLastReadPtr = dataPtr;
	  itsLastReadSize = dataLength;
	}
	return result;
      }
    }
  }
}      

Connection::State Connection::write()
{
  LOG_TRACE_FLOW("Connection write()");
  DBGASSERTSTR(itsSourceDH != 0, "No source DataHolder set!");
  DBGASSERTSTR(itsSourceDH->isInitialized(), 
	       "Source dataholder has not been initialized!");
  bool result = false;

  DBGASSERT(itsTransportHolder != 0);
  DBGASSERT(itsSourceDH != 0);

  LOG_TRACE_COND_STR("Transport::write; call send with tag " << getTag());

  itsSourceDH->pack();

  if (isBlocking())
  {
    result = getTransportHolder()->sendBlocking(itsSourceDH->getDataPtr(),
						itsSourceDH->getDataSize(),
						getTag(), itsSourceDH);
    if (result)
    {
      return Finished;
    }
    else
    {
      return Error;
    }
  } 
  else 
  {
    if (itsWriteState == Busy)
    {
      LOG_TRACE_COND("Previous nonblocking write has not yet finished. Waiting for this...");
      waitForWrite();      
    }
    result = getTransportHolder()->sendNonBlocking(itsSourceDH->getDataPtr(),
						   itsSourceDH->getDataSize(),
						   getTag(), itsSourceDH);

    if (result)
    {
      itsWriteState = Finished;
      return itsWriteState;
    }
    else
    {
      itsWriteState = Busy;
      return itsWriteState;
    }
  }
}


void Connection::waitForRead()
{
  if (!isBlocking())
  {
    switch (itsReadState)
    {
      case Idle:
        return;

      case TotalLength:
      {
	int msgLength = -1;
	getTransportHolder()->readTotalMsgLengthBlocking(getTag(), msgLength);	
	DBGASSERT(msgLength > 0);
	itsDestDH->resizeBuffer(msgLength);
	// Read total message
	void* dataPtr = itsDestDH->getDataPtr();
	bool result = getTransportHolder()->recvNonBlocking(dataPtr, msgLength,
							    getTag(), 0, itsDestDH);
	if (!result)
	{
	  getTransportHolder()->waitForReceived(dataPtr, msgLength, getTag());
	}
      }
      break;

      case Header:
      {
	// Wait for reading of header
	getTransportHolder()->waitForReceived(itsLastReadPtr, itsLastReadSize,
					      getTag());

	// Read rest of message
	uint dataLength = DataHolder::getDataLength(itsDestDH->getDataPtr()) - itsLastReadSize;
	itsDestDH->resizeBuffer(dataLength+itsLastReadSize);
	char* dataPtr = static_cast<char*>(itsDestDH->getDataPtr()) + itsLastReadSize;
	bool result = getTransportHolder()->recvNonBlocking(dataPtr, dataLength,
							    getTag(), itsLastReadSize, itsDestDH);
	if (!result)
	{
	  getTransportHolder()->waitForReceived(dataPtr, dataLength, getTag());	  
	}
      }
      break;

      case Message:
      {
	getTransportHolder()->waitForReceived(itsLastReadPtr, itsLastReadSize,
					      getTag());
      }
      break;
    }
    
    itsDestDH->unpack();
    itsReadState = Idle;
    itsLastReadPtr = 0;
    itsLastReadSize = 0;
  }
}

void Connection::waitForWrite()
{
  if (!isBlocking())
  {
    getTransportHolder()->waitForSent(itsSourceDH->getDataPtr(), 
				      itsSourceDH->getDataSize(), 
				      getTag());
  }
}

void Connection::setDestinationDH(DataHolder* dest)
{ 
  ASSERTSTR(itsReadState == Idle, "Cannot change the destination dataholder, connection is busy");
  itsDestDH = dest;
}
  
void Connection::setSourceDH(DataHolder* dest)
{ 
  ASSERTSTR(itsWriteState == Finished, "Cannot change the source dataholder, connection is busy");
  itsSourceDH = dest;
}


void Connection::dump() const 
{
  LOG_TRACE_FLOW("Connection dump()");
  cout << "Connection: Tag = " << getTag() << endl;
  if (itsSourceDH) {		// [REO]
    cout << " Source DataHolder = " << itsSourceDH->getName() << endl;
  }
  if (itsDestDH) {		// [REO]
    cout << " Destination DataHolder = " << itsDestDH->getName() << endl;
  }
  cout << " TransportHolder type = " << itsTransportHolder->getType() << endl;

}

} // end namespace
