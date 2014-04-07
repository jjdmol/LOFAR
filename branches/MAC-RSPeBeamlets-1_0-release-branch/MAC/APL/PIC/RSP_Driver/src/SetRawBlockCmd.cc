//#  SetRawBlockCmd.cc: implementation of the SetRawBlockCmd class
//#
//#  Copyright (C) 2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SetRawBlockCmd.h"

namespace LOFAR {
  namespace RSP {
//	using namespace blitz;
	using namespace RSP_Protocol;
	using namespace RTC;		// Timestamp

//
// SetRawBlockCmd(event, port, oper)
//
SetRawBlockCmd::SetRawBlockCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetRawBlock", port, oper)
{
	itsEvent = new RSPSetblockEvent(event);
}

//
// ~SetRawBlockCmd()
//
SetRawBlockCmd::~SetRawBlockCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void SetRawBlockCmd::ack(CacheBuffer& cache)
{
#if 1
	RSPSetblockackEvent ack;
	ack.timestamp = getTimestamp();
	ack.boardID	  = itsEvent->boardID;
	ack.status 	  = RSP_SUCCESS;

	getPort()->send(ack);
#endif
}

//
// apply(cache, setModFlag)
//
void SetRawBlockCmd::apply(CacheBuffer& cache, bool setModFlag)
{
	// fill the cache with the request and tickle it.
//	LOG_INFO(formatString("@@@SetRawBlockCmd::apply(%d,%0X,%d,%d,%s)", 
//			itsEvent->boardID, itsEvent->address, itsEvent->offset, itsEvent->dataLen, setModFlag ? "T" : "F"));

	// NOTE: [REO] I expected that the next 4 lines could also be in the if(setModFlag)
	//		 but it turned out that the info is than in the front cache when the RawDataBlockRead
	//		 command needs it!!!
	//		 Since only one command can be scheduled at the time, I write it in both caches.

	// copy information (always?) to the cache
	RawDataBlock_t&	rdb = cache.getRawDataBlock();
	rdb.address = itsEvent->address;
	rdb.offset  = itsEvent->offset;
	rdb.dataLen = itsEvent->dataLen;
	memcpy(rdb.data, itsEvent->data, rdb.dataLen);

//	string hDump;
//	hexdump(hDump, rdb.data, rdb.dataLen);
//	LOG_INFO(hDump);

	if (setModFlag) {
		// tell cache(status) that we expect a write action.
		cache.getCache().getState().rawdatawrite().write(itsEvent->boardID);
	}
}

//
// complete(cache)
//
void SetRawBlockCmd::complete(CacheBuffer& cache)
{
#if 0
	RSPSetblockackEvent ack;
	ack.timestamp = getTimestamp();
	ack.boardID	  = itsEvent->boardID;
	ack.status 	  = RSP_SUCCESS;

	getPort()->send(ack);
//	ack(cache);
#endif
}

//
// getTimeStamp()
//
const Timestamp& SetRawBlockCmd::getTimestamp() const
{
	return (itsEvent->timestamp);
}

//
// setTimestamp(timestamp)
//
void SetRawBlockCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool SetRawBlockCmd::validate() const
{
	return (true);
}

//
// ack_fail()
//
void SetRawBlockCmd::ack_fail()
{
	RSPSetblockackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status 	  = RSP_FAILURE;
	LOG_INFO ("SetRawBlockCmd::ack_fail");

	getPort()->send(ack);
}

  } // namepsace RSP
} // namespace LOFAR
