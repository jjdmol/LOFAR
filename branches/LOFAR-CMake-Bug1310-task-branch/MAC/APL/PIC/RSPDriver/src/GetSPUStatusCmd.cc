//#  GetSPUStatusCmd.cc: implementation of the GetSPUStatusCmd class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetSPUStatusCmd.h"

namespace LOFAR {
  namespace RSP {
//	using namespace blitz;
	using namespace RSP_Protocol;
	using namespace RTC;		// Timestamp

//
// GetSPUStatusCmd(event, port, oper)
//
GetSPUStatusCmd::GetSPUStatusCmd(GCFEvent& event, GCFPortInterface& port, Operation oper)
{
	itsEvent = new RSPGetspustatusEvent(event);

	setOperation(oper);
	setPeriod(0);
	setPort(port);
}

//
// ~GetSPUStatusCmd()
//
GetSPUStatusCmd::~GetSPUStatusCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void GetSPUStatusCmd::ack(CacheBuffer& cache)
{
	RSPGetspustatusackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = SUCCESS;
	ack.spustatus.subrack().resize(cache.getSPUStatus().subrack().size());
	ack.spustatus = cache.getSPUStatus();

	getPort()->send(ack);
}

//
// apply(cache, setModFlag)
//
void GetSPUStatusCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
	// no-op
}

//
// complete(cache)
//
void GetSPUStatusCmd::complete(CacheBuffer& cache)
{
	ack(cache);
}

//
// getTimeStamp()
//
const Timestamp& GetSPUStatusCmd::getTimestamp() const
{
	return (itsEvent->timestamp);
}

//
// setTimestamp(timestamp)
//
void GetSPUStatusCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool GetSPUStatusCmd::validate() const
{
	return (true);
}

//
// readFromCache()
//
bool GetSPUStatusCmd::readFromCache() const
{
	return (itsEvent->cache);
}

//
// ack_fail()
//
void GetSPUStatusCmd::ack_fail()
{
	RSPGetspustatusackEvent ack;

	ack.timestamp = Timestamp(0,0);
	ack.status = FAILURE;

	// send back dummy status array
	ack.spustatus.subrack().resize(1);
	SPUBoardStatus spustatusinit;
	memset(&spustatusinit, 0, sizeof(SPUBoardStatus));

	ack.spustatus.subrack()(0) = spustatusinit;

	getPort()->send(ack);
}

  } // namepsace RSP
} // namespace LOFAR
