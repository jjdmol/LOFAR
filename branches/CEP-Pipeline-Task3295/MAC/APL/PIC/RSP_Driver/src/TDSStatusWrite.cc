//#  TDSStatusWrite.cc: implementation of the TDSStatusWrite class
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

#include "TDSStatusWrite.h"
#include "TDSi2cdefs.h"
#include "Cache.h"

#include <netinet/in.h>


namespace LOFAR {
  namespace RSP {

using namespace EPA_Protocol;

static uint8 tds_readstatus[  TDS_READ_LOCKDETECT_SIZE
							+ TDS_READ_VOLT_SIZE
							+ TDS_READ_SPU_SIZE
							+ TDS_C_END_SIZE ] = {
	// read lock detect
	TDS_READ_LOCKDETECT,
	TDS_READ_VOLT,
	TDS_READ_SPU,
	TDS_C_END,
};

uint8 tds_readstatus_result[  TDS_READ_LOCKDETECT_RESULT_SIZE
							+ TDS_READ_VOLT_RESULT_SIZE
							+ TDS_READ_SPU_RESULT_SIZE
							+ TDS_C_END_RESULT_SIZE] = {
	0x00, 0x00,
	TDS_READ_VOLT_RESULT,
	TDS_READ_SPU_RESULT,
	0x00
};


//
// TDSStatusWrite(port, boardID)
//
TDSStatusWrite::TDSStatusWrite(GCFPortInterface& board_port, int board_id): 
	SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

//
// ~TDSStatusWrite()
//
TDSStatusWrite::~TDSStatusWrite()
{
	/* TODO: delete event? */
}

//
// sendrequest
//
void TDSStatusWrite::sendrequest()
{
	char* buf = 0;

	// always perform this action
	uint32 tds_control = 0;
	sscanf(GET_CONFIG_STRING("RSPDriver.TDS_CONTROL"), "%x", &tds_control);

	// only one RSPboard controls the TD, find out if this is the one.
	if (!(tds_control & (1 << getBoardId()))) {
		Cache::getInstance().getState().tdstatuswrite().write_ack(getBoardId());
		setContinue(true);
		return;
	}

	EPATdsProtocolEvent tdsprotocol;
	buf = (char*)tds_readstatus;
	tdsprotocol.hdr.set(MEPHeader::TDS_PROTOCOL_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE, sizeof(tds_readstatus), 0);
	tdsprotocol.protocol.setBuffer((char*)buf, sizeof(tds_readstatus));

	m_hdr = tdsprotocol.hdr; // remember header to match with ack

	getBoardPort().send(tdsprotocol);
}

//
// sendrequest_status()
//
void TDSStatusWrite::sendrequest_status()
{
	// intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult TDSStatusWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal) {
		LOG_WARN("TDSStatusWrite::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);		// convert to specific event.

	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("TDSStatusWrite::handleack: invalid ack");
		Cache::getInstance().getState().tdstatuswrite().write_error(getBoardId());
		return GCFEvent::NOT_HANDLED;
	}

	Cache::getInstance().getState().tdstatuswrite().write_ack(getBoardId());

	return GCFEvent::HANDLED;
}

  }; // namespace RSP
}; // namespace LOFAR
