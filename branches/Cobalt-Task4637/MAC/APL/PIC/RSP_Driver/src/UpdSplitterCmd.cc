//#  UpdSplitterCmd.cc: implementation of the UpdSplitterCmd class
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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetSplitterCmd.h"
#include "UpdSplitterCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

//
// UpdSplitterCmd(event, port, oper)
//
UpdSplitterCmd::UpdSplitterCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SubSplitter", port, oper)
{
	// convert event to SubSplitter event
	itsEvent = new RSPSubsplitterEvent(event);
	itsSplitterOn = -1;			// undefined yet

	setPeriod(itsEvent->period);
}

//
// ~UpdSplitterCmd()
//
UpdSplitterCmd::~UpdSplitterCmd()
{
	// delete our own event again
	delete itsEvent;
}

//
// ack(cache)
//
void UpdSplitterCmd::ack(CacheBuffer& /*cache*/)
{
	// intentionally left empty
}

//
// apply(cache)
//
void UpdSplitterCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
	// no-op
}

//
// complete(cache)
//
void UpdSplitterCmd::complete(CacheBuffer& cache)
{
	// check if we need to continue
//	LOG_INFO_STR("itsSplitterOn=" << itsSplitterOn << ", cache.isSplitterActive=" << (cache.isSplitterActive() ? 1 : 0));
	if (itsSplitterOn == (cache.isSplitterActive() ? 1 : 0)) {	// state didn't change?
		return;
	}

	itsSplitterOn = (cache.isSplitterActive() ? 1 : 0);	// copy settings

	// construct ack message
	RSPUpdsplitterEvent ack;
	ack.timestamp = getTimestamp();
	ack.status 	  = RSP_SUCCESS;
	ack.handle 	  = (memptr_t)this; // opaque ptr used to refer to the subscr.
	ack.splitter.reset();
	if (cache.isSplitterActive()) {
		for (int rsp = 0; rsp < StationSettings::instance()->nrRspBoards(); rsp++) {
			ack.splitter.set(rsp);
		}
	}

	// Finally send the answer
	getPort()->send(ack);
}

//
// getTimestamp()
//
const Timestamp& UpdSplitterCmd::getTimestamp() const
{
	return itsEvent->timestamp;
}

//
// setTimestamp(timestamp)
//
void UpdSplitterCmd::setTimestamp(const Timestamp& timestamp)
{
	itsEvent->timestamp = timestamp;
}

//
// validate()
//
bool UpdSplitterCmd::validate() const
{
	return (true);
}
