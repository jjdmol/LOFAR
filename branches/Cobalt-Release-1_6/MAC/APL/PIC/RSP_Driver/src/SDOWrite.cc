//#  SDOWrite.cc: implementation of the SDOWrite class
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
//#  $Id: SDOWrite.cc 22585 2012-10-31 10:29:43Z mol $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SDOWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SDOWrite::SDOWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, NR_BLPS_PER_RSPBOARD * (MAX_BITS_PER_SAMPLE / MIN_BITS_PER_SAMPLE)),
    itsActiveBanks(1)
    
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
    doAtInit();
}

SDOWrite::~SDOWrite()
{
	/* TODO: delete event? */
}

void SDOWrite::sendrequest()
{
    if (StationSettings::instance()->hasAartfaac() == false) {
        LOG_INFO_STR(formatString("SDOWrite:: No Aartfaac on this station"));
        //Cache::getInstance().getState().sdoSelectState().unmodified(global_blp);
        setContinue(true); // continue with next action
        setFinished();
        return;
    }

    itsActiveBanks   = (MAX_BITS_PER_SAMPLE / Cache::getInstance().getBack().getSDOBitsPerSample());
	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + (getCurrentIndex() / itsActiveBanks);
	
    if (getCurrentIndex() >= (itsActiveBanks * NR_BLPS_PER_RSPBOARD)) {
		//Cache::getInstance().getState().sdoSelectState().unmodified(global_blp);
        setContinue(true);
        //setFinished();
		return;
	}
    
    if (RTC::RegisterState::WRITE != Cache::getInstance().getState().sdoSelectState().get(global_blp))
    {
        Cache::getInstance().getState().sdoSelectState().unmodified(global_blp);
        setContinue(true);
        return;
    }
        
    
    LOG_DEBUG(formatString(">>>> SDOWrite(%s) global_blp=%d", getBoardPort().getName().c_str(), global_blp));
	
    // mark modified
	//Cache::getInstance().getState().sdoSelectState().write_now(global_blp);

	// send subband select message
	EPASdoSelectEvent sdo;

	int dstid = 1 << (getCurrentIndex() / itsActiveBanks);
	// used bank 
	int bank = getCurrentIndex() % itsActiveBanks;
    
	LOG_DEBUG(formatString("SDOWRITE:board=%d, index=%d, globalblp=%d, dstID=%d, bank=%d, regid=%d", 
							getBoardId(), getCurrentIndex(), global_blp, dstid, bank, MEPHeader::SDO_SELECT+bank));

	sdo.hdr.set( MEPHeader::WRITE, 
				 dstid,
				 MEPHeader::SDO,
				 MEPHeader::SDO_SELECT+bank,
				 MEPHeader::SDO_SELECT_SIZE);

	// create array to contain the subband selection
	Array<uint16, 2> subbands((uint16*)&sdo.subbands,
							shape(MEPHeader::N_SDO_SUBBANDS, N_POL),
							neverDeleteData);
    
    //LOG_INFO_STR("SDOWRITE:subbands x=" << Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2,     bank, Range::all()));
    //LOG_INFO_STR("SDOWRITE:subbands y=" << Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2+1,   bank, Range::all()));
    subbands(Range::all(), 0) = Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2,     bank, Range::all()); // x
    subbands(Range::all(), 1) = Cache::getInstance().getBack().getSDOSelection().subbands()(global_blp * 2 + 1, bank, Range::all()); // y
    LOG_DEBUG_STR("SDOWRITE:subbands =" << subbands(Range::all(), Range::all()));
	m_hdr = sdo.hdr;
	getBoardPort().send(sdo);
}

void SDOWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult SDOWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal) {
		LOG_WARN("SDOWrite::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);

	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + (getCurrentIndex()/itsActiveBanks);

	if (!ack.hdr.isValidAck(m_hdr)) {
		Cache::getInstance().getState().sdoSelectState().write_error(global_blp);

		LOG_ERROR("SDOWrite::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}
	
    if ((getCurrentIndex() % itsActiveBanks) == (itsActiveBanks - 1)) {
        LOG_DEBUG_STR("SDOwrite::ack.status=" << ack);
	    Cache::getInstance().getState().sdoSelectState().write_ack(global_blp);
	}

	return GCFEvent::HANDLED;
}
