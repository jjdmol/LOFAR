//#  GetDatastreamCmd.cc: implementation of the GetDatastreamCmd class
//#
//#  Copyright (C) 2009
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
//#  $Id: GetDatastreamCmd.cc 14660 2009-12-10 13:33:18Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetDatastreamCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;


//
// GetDatastreamCmd(event, port, oper)
//
GetDatastreamCmd::GetDatastreamCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetDatastream", port, oper)
{
	itsEvent = new RSPGetdatastreamEvent(event);
}

//
// ~SetSPlitterCmd()
//
GetDatastreamCmd::~GetDatastreamCmd()
{
	delete itsEvent;
}

//
// ack(cache)
//
void GetDatastreamCmd::ack(CacheBuffer& cache)
{
	complete(cache);
	// moved code to the complete function so that the response is
	// sent back after it was applied.
}

//
// apply(cache, setMod);
//
void GetDatastreamCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
    // empty
}

//
// complete(cache)
//
void GetDatastreamCmd::complete(CacheBuffer& cache)
{
	RSPGetdatastreamackEvent ack;
	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;
	ack.switch_on = cache.isCepEnabled();
	getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& GetDatastreamCmd::getTimestamp() const
{
	return itsEvent->timestamp;
}

//
// setTimestamp(timestamp)
//
void GetDatastreamCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool GetDatastreamCmd::validate() const
{
	return (true);
}
