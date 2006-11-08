//# CSConnection.cc:
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

#include <lofar_config.h>

#include <Transport/CSConnection.h>
#include <Transport/DataHolder.h>
#include <Transport/TransportHolder.h>
#include <Common/LofarLogger.h>
#include <stdlib.h>
#include <memory.h>


namespace LOFAR
{
int CSConnection::theirNextTag = 0;

CSConnection::CSConnection (const string& name, DataHolder* dhSource, DataHolder* dhDest,
			TransportHolder* th, bool blockingComm)
  : itsName            (name), 
    itsSourceDH        (dhSource),
    itsDestDH          (dhDest),
    itsTransportHolder (th),
    itsIsBlocking      (blockingComm),
    itsReadState       (Idle),
    itsReadOffset      (0),
    itsReadSize        (0),
    itsWriteState      (Finished)
{
  LOG_TRACE_FLOW("CSConnection constructor");

  if (itsSourceDH && itsDestDH) {
    ASSERTSTR(dhSource->getType() == dhDest->getType(), 
	    "Connected DataHolders should be of the same type.");
    ASSERTSTR(dhSource->getVersion() == dhDest->getVersion(),
	    "Connected DataHolders should have the same version number");
  }

  itsTag = theirNextTag++;

  if (itsSourceDH && !itsSourceDH->isInitialized()) {
LOG_TRACE_FLOW("CSConnection itsSourceDH settingAllocationProperties");
    itsSourceDH->setAllocationProperties(th->canDataGrow(), th->blobStringType());
  }
  if (itsDestDH && !itsDestDH->isInitialized()) {
LOG_TRACE_FLOW("CSConnection itsDestDH settingAllocationProperties");
    itsDestDH->setAllocationProperties(th->canDataGrow(), th->blobStringType());
  }
}

CSConnection::~CSConnection()
{
  LOG_TRACE_FLOW("CSConnection destructor");
}

CSConnection::State CSConnection::read()
{
  LOG_TRACE_RTTI("CSConnection read()");
  DBGASSERTSTR(itsDestDH != 0, "No destination DataHolder set!");
  DBGASSERTSTR(itsDestDH->isInitialized(), 
	       "Destination dataholder has not been initialized!");
  DBGASSERT(getTransportHolder() != 0);
  DBGASSERT(itsDestDH != 0);

  LOG_TRACE_COND_STR("Transport::read; call recv with tag " << getTag());

  // blocking
  if (isBlocking()) {
    if (!readBlocking()) {
      return Error;
    }
  }
  else { // nonblocking
    if (!readNonBlocking()) {
      return Busy;
    }
  }

  itsDestDH->unpack();
  return Finished;

}


//
// readMessagePart(blocking) [private]
//
// Basic algorithm for reading a message can be split in three states:
//
// [TotalLength]
// TH->readTotalMsgLength (&ml)		some TH's support this call
// if (ml <= 0) {
//    [Header]
//    TH->recv(DH->getHeaderSize())	not supported, try it this way
//    ml = DH->getDataSize()
// }
// [Message]
// DH->resizeBuffer(ml)			finally read the (rest of the) msg
// TH->recv(ml)
//
bool CSConnection::readMessagePart(bool	blocking)
{
	for(;;) {
  		LOG_TRACE_COND_STR("CSConnection:readMessagePart:" << itsReadState 
						<< "," << itsReadSize << "," << itsReadOffset);

		switch (itsReadState) {
		case Idle: {
			// Start with new message
			if (itsDestDH->hasFixedSize()) {    
				itsReadOffset = 0;
				itsReadSize   = itsDestDH->getDataSize();
				itsReadState  = Message;
			}
			else { // not fixedSize, try TotalLength first
				itsReadOffset = 0;
				itsReadSize   = 0;
				itsReadState  = TotalLength;
			}
			// continue with next state
		}
		break;

		case TotalLength: {
			// Try to get the totalLength of the message
			int msgLength = -1;
			if (blocking) {
				getTransportHolder()->readTotalMsgLengthBlocking(
														getTag(), msgLength);
			}
			else { // nonblocking
				if (!getTransportHolder()->
						readTotalMsgLengthNonBlocking(getTag(), msgLength)) {
					// Information not yet available, try again next time
					return (false);
				}
			}

			if (msgLength > 0) {	// Yes, got length continue with message
				itsDestDH->resizeBuffer(msgLength);
				itsReadOffset = 0;
				itsReadSize   = msgLength;
				itsReadState  = Message;
				// continue with next state
				break;
			}

			// Information not available for this typeof TH, read header first.
			itsReadOffset = 0;
			itsReadSize   = itsDestDH->getHeaderSize();
			itsReadState  = Header;
			// continue with next state
		}
		break;

		case Header: {
			// Try to read the header of the message
			int32 nrBytesRead;
			if (blocking) {
				if (!getTransportHolder()->recvBlocking(
					static_cast<char*>(itsDestDH->getDataPtr())+itsReadOffset, 
					itsReadSize, getTag(), itsReadOffset, itsDestDH)) {
					// reading of complete message failed
					itsReadState = Idle;			// flush garbage
					return (false);			// return failure
				}
				nrBytesRead = itsReadSize;		// otherwise false was returned
			}
			else { // nonblocking
				nrBytesRead = getTransportHolder()->recvNonBlocking(
					static_cast<char*>(itsDestDH->getDataPtr())+itsReadOffset, 
					itsReadSize, getTag(), itsReadOffset, itsDestDH);
			}

			// update internal admin
			itsReadOffset += nrBytesRead;
			itsReadSize   -= nrBytesRead;

			// header not yet complete?
			if (itsReadSize > 0) {
				return (false);
			}

			// header complete, diddle out the message length
			int32 totalMsgSize = DataHolder::getDataLength(itsDestDH->getDataPtr());
			itsDestDH->resizeBuffer(totalMsgSize);

			itsReadSize  = totalMsgSize - itsReadOffset;
			itsReadState = Message;
			// continue with next state
		}
		break;

		case Message: {
			// Try to read the data of the message
			int32 nrBytesRead;
			if (blocking) {
				if (!getTransportHolder()->recvBlocking(
					static_cast<char*>(itsDestDH->getDataPtr())+itsReadOffset, 
					itsReadSize, getTag(), itsReadOffset, itsDestDH)) {
					// reading of complete message failed
					itsReadState = Idle;			// flush garbage
					return (false);			// return failure
				}
				nrBytesRead = itsReadSize;		// otherwise false was returned
			}
			else { // nonblocking
				nrBytesRead = getTransportHolder()->recvNonBlocking(
					static_cast<char*>(itsDestDH->getDataPtr())+itsReadOffset, 
					itsReadSize, getTag(), itsReadOffset, itsDestDH);
			}

			// update internal admin
			itsReadOffset += nrBytesRead;
			itsReadSize   -= nrBytesRead;

			// message not yet complete?
			if (itsReadSize > 0) {
				return (false);
			}

			// Cool, message is complete! 
			itsDestDH->unpack();
			itsReadState  = Idle;	// in case waitForRead is called.
			itsReadOffset = 0;
			itsReadSize   = 0;
			return (true);		// finally!
		}
	    break;
		} // switch
	} // for ever 

	DBGASSERTSTR(false, 
		"Illegal state " << itsReadState << "in CSConnection:readMessagePart");
}

//
// readBlocking() [private]
//
// We always want to read a complete message at once.
//
bool CSConnection::readBlocking()
{
  LOG_TRACE_COND("CSConnection:readBlocking");

  itsReadState = Idle;			// Force new message
  return (readMessagePart(true));
}

//
// readNonBlocking() [private]
//
// Might be called repeatedly until complete message is received.
//
bool CSConnection::readNonBlocking()
{
  LOG_TRACE_COND("CSConnection:readNonBlocking");

  return (readMessagePart(false));
}

//
// waitForRead()
//
void CSConnection::waitForRead()
{
  LOG_TRACE_COND("CSConnection:waitForRead");

  if (isBlocking() || (itsReadState == Idle)) {
    return;
  }

  while (!readMessagePart(false)) {	// loop until complete msg received.
    ;
  }
}


CSConnection::State CSConnection::write()
{
  LOG_TRACE_FLOW("CSConnection write()");
  DBGASSERTSTR(itsSourceDH != 0, "No source DataHolder set!");
  DBGASSERTSTR(itsSourceDH->isInitialized(), 
	       "Source dataholder has not been initialized!");

  DBGASSERT(itsTransportHolder != 0);
  DBGASSERT(itsSourceDH != 0);

  LOG_TRACE_COND_STR("Transport::write; call send with tag " << getTag());

  itsSourceDH->pack();

  if (isBlocking()) {
    if (getTransportHolder()->sendBlocking(itsSourceDH->getDataPtr(),
						itsSourceDH->getDataSize(),
						getTag(), itsSourceDH)) {
      return Finished;
    }
    return Error;
  } 

  // nonblocking
  if (itsWriteState == Busy) {
    LOG_TRACE_COND("Previous nonblocking write has not yet finished. Waiting for this...");
    waitForWrite();      
  }

  if (getTransportHolder()->sendNonBlocking(itsSourceDH->getDataPtr(),
					   itsSourceDH->getDataSize(),
					   getTag(), itsSourceDH)) {
    itsWriteState = Finished;
    return itsWriteState;
  }

  itsWriteState = Busy;
  return itsWriteState;
}


void CSConnection::waitForWrite()
{
  if (!isBlocking())
  {
    getTransportHolder()->waitForSent(itsSourceDH->getDataPtr(), 
				      itsSourceDH->getDataSize(), 
				      getTag());
  }
}

void CSConnection::setDestinationDH(DataHolder* dest)
{ 
  ASSERTSTR(itsReadState == Idle, "Cannot change the destination dataholder, connection is busy");
  itsDestDH = dest;
}
  
void CSConnection::setSourceDH(DataHolder* dest)
{ 
  ASSERTSTR(itsWriteState == Finished, "Cannot change the source dataholder, connection is busy");
  itsSourceDH = dest;
}


void CSConnection::dump() const 
{
  LOG_TRACE_FLOW("CSConnection dump()");
  cout << "CSConnection: Tag = " << getTag() << endl;
  if (itsSourceDH) {
    cout << " Source DataHolder = " << itsSourceDH->getName() << endl;
  }
  if (itsDestDH) {
    cout << " Destination DataHolder = " << itsDestDH->getName() << endl;
  }
  cout << " TransportHolder type = " << itsTransportHolder->getType() << endl;

}

} // end namespace
