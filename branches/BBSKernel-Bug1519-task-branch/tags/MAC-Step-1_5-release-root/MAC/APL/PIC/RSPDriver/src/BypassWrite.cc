//#  BypassWrite.cc: implementation of the BypassWrite class
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
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "BypassWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;


BypassWrite::BypassWrite(GCFPortInterface&	board_port, 
						 int 				board_id,
						 int				bpNr)
  : SyncAction(board_port, board_id, 1),
	itsBPNr	  (bpNr)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

BypassWrite::~BypassWrite()
{
}

void BypassWrite::sendrequest()
{
	uint16	globalBP = getBoardId() * StationSettings::instance()->nrBlpsPerBoard() + itsBPNr;

	// cache modified, or initialising
	if (Cache::getInstance().getState().bypasssettings().get(globalBP) !=
															RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().bypasssettings().unmodified(globalBP);
		setContinue(true);
		return;
	}

	EPADiagBypassEvent request;

	request.hdr.set(MEPHeader::DIAG_BYPASS_HDR, 1 << itsBPNr, MEPHeader::WRITE);

	// read values from cache
	BypassSettings& s = Cache::getInstance().getBack().getBypassSettings(); // whole array
	request.bypass = s()(globalBP).getRaw();		// one element

	m_hdr = request.hdr;
	LOG_DEBUG(formatString("BypassWrite: sendrequest: sending it(%04X)", request.bypass));
	getBoardPort().send(request);
}

void BypassWrite::sendrequest_status()
{
// intentionally left empty
}

GCFEvent::TResult BypassWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	uint16	globalBP = getBoardId() * StationSettings::instance()->nrBlpsPerBoard() + itsBPNr;

	if (event.signal != EPA_WRITEACK) {
		LOG_WARN("BypassWrite::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);

	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("BypassWrite::handleack: invalid ack");
		Cache::getInstance().getState().bypasssettings().write_error(globalBP);
		return GCFEvent::NOT_HANDLED;
	}

	LOG_DEBUG_STR("BypassWrite: handleack: bp= " << globalBP);
	Cache::getInstance().getState().bypasssettings().write_ack(globalBP); // mark finished

	return GCFEvent::HANDLED;
}


