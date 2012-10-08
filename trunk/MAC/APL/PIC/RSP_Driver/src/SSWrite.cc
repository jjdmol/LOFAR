//#  SSWrite.cc: implementation of the SSWrite class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SSWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SSWrite::SSWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, NR_BLPS_PER_RSPBOARD * MAX_BITS_PER_SAMPLE / MIN_BITS_PER_SAMPLE)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

SSWrite::~SSWrite()
{
	/* TODO: delete event? */
}

void SSWrite::sendrequest()
{
	itsActiveBanks = (MAX_BITS_PER_SAMPLE / Cache::getInstance().getBack().getBitsPerSample());
	if (getCurrentIndex() >= (itsActiveBanks*NR_BLPS_PER_RSPBOARD)) {
		setContinue(true);
		return;
	}
	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + (getCurrentIndex()/itsActiveBanks);
	LOG_DEBUG(formatString(">>>> SSWrite(%s) global_blp=%d", getBoardPort().getName().c_str(), global_blp));

	// mark modified
	//Cache::getInstance().getState().ss().write_now(global_blp);

	// send subband select message
	EPASsSelectEvent ss;

	int dstid = 1 << (getCurrentIndex() / itsActiveBanks);
	// used bank 
	int bank = getCurrentIndex() % itsActiveBanks;
	LOG_INFO(formatString("SSWRITE:board=%d, index=%d, globalblp=%d, dstID=%d, bank=%d, regid=%d", 
							getBoardId(), getCurrentIndex(), global_blp, dstid, bank, MEPHeader::SS_SELECT+bank));

	ss.hdr.set( MEPHeader::WRITE, 
				dstid,
				MEPHeader::SS,
				MEPHeader::SS_SELECT+bank,
				MEPHeader::SS_SELECT_SIZE);

	// create array to contain the subband selection
	Array<uint16, 2> subbands((uint16*)&ss.subbands,
							shape(MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS, N_POL),
							neverDeleteData);

	// copy crosslet selection
	Range xlet_range(0, MEPHeader::N_LOCAL_XLETS-1);
	subbands(xlet_range, 0) = Cache::getInstance().getBack().getSubbandSelection().crosslets()(global_blp * 2,     bank, xlet_range); // x
	subbands(xlet_range, 1) = Cache::getInstance().getBack().getSubbandSelection().crosslets()(global_blp * 2 + 1, bank, xlet_range); // y

	//
	// copy the actual values from the cache
	// Explain this in more detail
	for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {

		int hw_offset = lane + MEPHeader::N_LOCAL_XLETS;
		int cache_offset = (lane * (MEPHeader::N_BEAMLETS / MEPHeader::N_SERDES_LANES));

		// strided source range, stride = NR_BLPS_PER_RSPBOARD
		Range hw_range(hw_offset, hw_offset + MEPHeader::N_BEAMLETS - MEPHeader::N_BLPS, MEPHeader::N_BLPS);
		Range cache_range(cache_offset, cache_offset + (MEPHeader::N_BEAMLETS / MEPHeader::N_SERDES_LANES) - 1, 1);

		LOG_DEBUG_STR("lane=" << lane << ",hw_range=" << hw_range << ",cache_range=" << cache_range);

		subbands(hw_range, 0) = Cache::getInstance().getBack().getSubbandSelection().beamlets()(global_blp * 2,     bank, cache_range); // x
		subbands(hw_range, 1) = Cache::getInstance().getBack().getSubbandSelection().beamlets()(global_blp * 2 + 1, bank, cache_range); // y
	}

	m_hdr = ss.hdr;
//  LOG_INFO_STR("SUBBANDSELECT=" << subbands);
	getBoardPort().send(ss);
}

void SSWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult SSWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal) {
		LOG_WARN("SSWrite::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);

	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + (getCurrentIndex()/itsActiveBanks);

	if (!ack.hdr.isValidAck(m_hdr)) {
		Cache::getInstance().getState().ss().write_error(global_blp);

		LOG_ERROR("SSWrite::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	Cache::getInstance().getState().ss().write_ack(global_blp);

	return GCFEvent::HANDLED;
}
