//#  BypassRead.cc: implementation of the BypassRead class
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

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "BypassRead.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BypassRead::BypassRead(GCFPortInterface& board_port, int board_id, int	bpNr)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard()),
	itsBPNr	  (bpNr)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

BypassRead::~BypassRead()
{
}

void BypassRead::sendrequest()
{
	// Message per rcu.
	EPAReadEvent bypassread;
	bypassread.hdr.set(MEPHeader::DIAG_BYPASS_HDR, itsBPNr << getCurrentIndex(), MEPHeader::READ);

	m_hdr = bypassread.hdr;
	getBoardPort().send(bypassread);
}

void BypassRead::sendrequest_status()
{
	/* intentionally left empty */
}

GCFEvent::TResult BypassRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	// check event type
	if (event.signal != EPA_DIAG_BYPASS) {
		LOG_WARN("BypassRead::handleack: unexpected ack");
		return (GCFEvent::NOT_HANDLED);
	}

	// check header contents
	EPADiagBypassEvent bypassAckMsg(event);
	if (!bypassAckMsg.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("BypassRead::handleack: invalid ack");
		return (GCFEvent::NOT_HANDLED);
	}

	uint16 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) 
						+ getCurrentIndex();

	LOG_DEBUG_STR("BypassRead: handleack: bp= " << global_blp);
	Cache::getInstance().getBack().getBypassSettings()()(global_blp).setRaw(bypassAckMsg.bypass);

	return (GCFEvent::HANDLED);
}
