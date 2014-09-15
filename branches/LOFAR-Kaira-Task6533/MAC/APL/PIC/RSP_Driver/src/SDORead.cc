//#  SDORead.cc: implementation of the SDORead class
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
//#  $Id: SDORead.cc 22585 2012-10-31 10:29:43Z mol $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarBitModeInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SDORead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SDORead::SDORead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, NR_BLPS_PER_RSPBOARD*(MAX_BITS_PER_SAMPLE/MIN_BITS_PER_SAMPLE))
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

SDORead::~SDORead()
{
}

void SDORead::sendrequest()
{
	if (StationSettings::instance()->hasAartfaac() == false) {
        LOG_DEBUG_STR(formatString("SDORead:: No Aartfaac on this station", getBoardId()));
        Cache::getInstance().getState().sdoSelectState().unmodified(getBoardId());
        setContinue(true); // continue with next action
        setFinished();
        return;
    }
    
    EPAReadEvent sdoread;
	itsActivePlanes = (MAX_BITS_PER_SAMPLE / Cache::getInstance().getBack().getSDOBitsPerSample());
	if (getCurrentIndex() >= (itsActivePlanes*NR_BLPS_PER_RSPBOARD)) {
		setContinue(true);
		return;
	}

	int dstid = 1 << (getCurrentIndex() / itsActivePlanes);
	int plane = getCurrentIndex() % itsActivePlanes;

	sdoread.hdr.set(MEPHeader::READ, 
					dstid,
					MEPHeader::SDO,
					MEPHeader::SDO_SELECT+plane,
					MEPHeader::SDO_SELECT_SIZE);

	m_hdr = sdoread.hdr;
	getBoardPort().send(sdoread);
}

void SDORead::sendrequest_status()
{
	/* intentionally left empty */
}

GCFEvent::TResult SDORead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if ((event.signal < EPA_SDO_SELECT) || (event.signal > (EPA_SDO_SELECT+3))) {
		LOG_WARN("SDORead::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	// unpack sdo message
	EPASdoSelectEvent sdo(event);

	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + (getCurrentIndex()/itsActivePlanes);
	if (!sdo.hdr.isValidAck(m_hdr)) {
		Cache::getInstance().getState().sdoSelectState().read_error(global_blp);
		LOG_ERROR("SDORead::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	LOG_DEBUG("handleack");

	LOG_INFO(formatString(">>>> SDORead(%s) global_blp=%d",
	getBoardPort().getName().c_str(), global_blp));

	// create array point to data in the response event (format in 2 dims)
	Array<uint16, 2> subbands((uint16*)&sdo.subbands, 
							shape(MEPHeader::N_SDO_SUBBANDS, N_POL),
							neverDeleteData);

	Range hw_range;
	// used plane 
	int plane = getCurrentIndex()%itsActivePlanes;
	if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i)) {
		subbands(Range::all(), 0) -= 
				Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2,     plane, Range::all());
		subbands(Range::all(), 1) -= 
				Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2 + 1, plane, Range::all());

		uint16 sdosum = sum(subbands);

		if (0 != sdosum) {
			LOG_WARN(formatString("LOOPBACK CHECK FAILED: SDORead mismatch (blp=%d, error=%d)", global_blp, sdosum));
		}
	}
	else {
		// copy into the cache
		Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2, plane, Range::all())
				= subbands(Range::all(), 0); // x
		Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2 + 1, plane, Range::all())
				= subbands(Range::all(), 1); // y
	}

	Cache::getInstance().getState().sdoSelectState().read_ack(global_blp);

	return GCFEvent::HANDLED;
}
