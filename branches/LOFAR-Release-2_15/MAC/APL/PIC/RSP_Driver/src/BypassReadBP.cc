//#  BypassReadBP.cc: implementation of the BypassReadBP class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: BypassReadBP.cc 30919 2015-02-05 15:26:22Z amesfoort $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "BypassReadBP.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BypassReadBP::BypassReadBP(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1 /* BP */)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

BypassReadBP::~BypassReadBP()
{
}

void BypassReadBP::sendrequest()
{
	// Message per rcu.
	EPAReadEvent bypassread;
	bypassread.hdr.set(MEPHeader::DIAG_BYPASS_HDR, MEPHeader::DST_RSP, MEPHeader::READ);

	m_hdr = bypassread.hdr;
	getBoardPort().send(bypassread);
}

void BypassReadBP::sendrequest_status()
{
	/* intentionally left empty */
}

GCFEvent::TResult BypassReadBP::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	// check event type
	if (event.signal != EPA_DIAG_BYPASS) {
		LOG_WARN("BypassReadBP::handleack: unexpected ack");
		return (GCFEvent::NOT_HANDLED);
	}

	// check header contents
	EPADiagBypassEvent bypassAckMsg(event);
	if (!bypassAckMsg.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("BypassReadBP::handleack: invalid ack");
		return (GCFEvent::NOT_HANDLED);
	}
    
    LOG_DEBUG_STR("BypassReadBP: handleack: board= " << getBoardId());
    Cache::getInstance().getBack().getBypassSettingsBP()()(getBoardId()).setRaw(bypassAckMsg.bypass);
	return (GCFEvent::HANDLED);
}
