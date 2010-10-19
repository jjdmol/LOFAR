//#  GetRawBlockCmd.cc: implementation of the GetRawBlockCmd class
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
#include "GetRawBlockCmd.h"

namespace LOFAR {
  namespace RSP {
//	using namespace blitz;
	using namespace RSP_Protocol;
	using namespace RTC;		// Timestamp

//
// GetRawBlockCmd(event, port, oper)
//
GetRawBlockCmd::GetRawBlockCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
	itsEvent = new RSPGetblockEvent(event);

	setOperation(oper);
	setPeriod(0);
	setPort(port);
}

//
// ~GetRawBlockCmd()
//
GetRawBlockCmd::~GetRawBlockCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void GetRawBlockCmd::ack(CacheBuffer& cache)
{
	RSPGetblockackEvent ack;
	RawDataBlock_t&		rdb = cache.getRawDataBlock();
	ack.timestamp = getTimestamp();
	ack.boardID	  = itsEvent->boardID;
	ack.status 	  = SUCCESS;
	ack.dataLen   = rdb.dataLen;
	memcpy(ack.data, rdb.data, ack.dataLen);

	getPort()->send(ack);
}

//
// apply(cache, setModFlag)
//
void GetRawBlockCmd::apply(CacheBuffer& cache, bool setModFlag)
{
	// fill the cache with the request and tickle it.
//	LOG_INFO(formatString("@@@GetRawBlockCmd::apply(%d,%0X,%d,%d)", 
//			itsEvent->boardID, itsEvent->address, itsEvent->offset, itsEvent->dataLen));

	// NOTE: [REO] I expected that the next 4 lines could also be in the if(setModFlag)
	//		 but it turned out that the info is than in the front cache when the RawDataBlockRead
	//		 command needs it!!!
	//		 Since only one command can be scheduled at the time, I write it in both caches.

	// copy information (always?) to the cache
	RawDataBlock_t&	rdb = cache.getRawDataBlock();
	rdb.address = itsEvent->address;
	rdb.offset  = itsEvent->offset;
	rdb.dataLen = itsEvent->dataLen;

	if (setModFlag) {
		// tell cache(status) that we expect a write(!) action.
		cache.getCache().getState().rawdataread().write(itsEvent->boardID);
	}
}

//
// complete(cache)
//
void GetRawBlockCmd::complete(CacheBuffer& cache)
{
	ack(cache);
}

//
// getTimeStamp()
//
const Timestamp& GetRawBlockCmd::getTimestamp() const
{
	return (itsEvent->timestamp);
}

//
// setTimestamp(timestamp)
//
void GetRawBlockCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool GetRawBlockCmd::validate() const
{
	return (true);
}

//
// readFromCache()
//
bool GetRawBlockCmd::readFromCache() const
{
	return (false);
}

//
// ack_fail()
//
void GetRawBlockCmd::ack_fail()
{
	RSPGetblockackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status 	  = FAILURE;
	ack.dataLen   = 0;
	LOG_INFO ("GetRawBlockCmd::ack_fail");

	getPort()->send(ack);
}

  } // namepsace RSP
} // namespace LOFAR
