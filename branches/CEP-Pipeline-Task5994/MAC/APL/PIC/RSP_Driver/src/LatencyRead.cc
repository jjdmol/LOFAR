//#  LatencyRead.cc: implementation of the TDSStatusRead class
//#
//#  Copyright (C) 2002-2004
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
#include <Common/LofarConstants.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "LatencyRead.h"
#include "Cache.h"
#include "StationSettings.h"

#include <netinet/in.h>
#include <blitz/array.h>

namespace LOFAR {
  namespace RSP {
	using namespace EPA_Protocol;
	using namespace blitz;
	using namespace RTC;

//
// LatencyRead(port, boardID)
//
LatencyRead::LatencyRead(GCFPortInterface& board_port, int board_id): 
	SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

//
// ~LatencyRead()
//
LatencyRead::~LatencyRead()
{
	/* TODO: delete event? */
}

//
// sendrequest()
//
void LatencyRead::sendrequest()
{
	// send read event
	EPAReadEvent latencyread;
	latencyread.hdr.set(MEPHeader::RAD_LATENCY_HDR, MEPHeader::DST_RSP, MEPHeader::READ);
	m_hdr = latencyread.hdr; // remember header to match with ack
	getBoardPort().send(latencyread);
}

//
// sendrequest_status
//
void LatencyRead::sendrequest_status()
{
	// intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult LatencyRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	// is this the message type we are waiting for?
	if (EPA_RAD_LAT != event.signal) {
		LOG_WARN("LatencyRead::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	// convert to TDSResultEvent
	EPARadLatEvent ack(event);
	
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("LatencyRead::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	Cache::getInstance().getBack().getLatencys()()(getBoardId()) = ack.latency;
	
	return GCFEvent::HANDLED;
}

  } // namespace RSP
} // namespace LOFAR
