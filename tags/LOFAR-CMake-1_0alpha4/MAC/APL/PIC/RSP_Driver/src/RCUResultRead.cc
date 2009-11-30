//#  RCUResultRead.cc: implementation of the RCUResultRead class
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
#include <Common/hexdump.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "RCUProtocolWrite.h"
#include "RCUResultRead.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

RCUResultRead::RCUResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrRcusPerBoard())
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUResultRead::~RCUResultRead()
{
	/* TODO: delete event? */
}

// Note: this class is called when the rcumode was written and when a rcumode read command was send.
//	The write sequence is managed by the rcuprotocol states.
//	The read  sequence is managed by the rcuread states.
//	The write sequence expects 6 bytes result with the mode on places 1,2,3
//	The read  sequence expects 5 bytes result with the mode on places 0,1,2
//	Although it is not possible to schedule a write and a read sequence in the same second we always
//	prefer the write sequence. If the write sequence is NOT active than we test is the there is a read 
//	sequence going on.

//
// sendrequest()
//
void RCUResultRead::sendrequest()
{
	uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();
	bool	handlingWriteResult(Cache::getInstance().getState().rcuprotocol().get(global_rcu) == RTC::RegisterState::READ);

	// skip update if the RCU settings have not been applied yet
	if (Cache::getInstance().getState().rcuprotocol().get(global_rcu) != RTC::RegisterState::READ &&
		Cache::getInstance().getState().rcuread().get(global_rcu) 	  != RTC::RegisterState::READ) {
		setContinue(true);
		return;
	}

	// set appropriate header
	MEPHeader::FieldsType hdr;
	if (0 == global_rcu % MEPHeader::N_POL) {
		hdr = MEPHeader::RCU_RESULTX_HDR;
	} 
	else {
		hdr = MEPHeader::RCU_RESULTY_HDR;
	}

	EPAReadEvent rcuresult;
	rcuresult.hdr.set(hdr, 1 << (getCurrentIndex() / MEPHeader::N_POL), MEPHeader::READ, 
						handlingWriteResult ? RCUProtocolWrite::RESULT_WRITE_SIZE : RCUProtocolWrite::RESULT_READ_SIZE);

	m_hdr = rcuresult.hdr; // remember header to match with ack
	getBoardPort().send(rcuresult);
}

//
// sendrequest_status()
//
void RCUResultRead::sendrequest_status()
{
  // intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult RCUResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_RCU_RESULT != event.signal) {
		LOG_WARN("RCUResultRead::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	uint8	global_rcu 			= (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();
	bool	handlingWriteResult = (Cache::getInstance().getState().rcuprotocol().get(global_rcu) == RTC::RegisterState::READ);
	int		resultOffset		= handlingWriteResult ? 1 : 0;

	EPARcuResultEvent ack(event);
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("RCUResultRead::handleack: invalid ack");
		if (handlingWriteResult) {
			Cache::getInstance().getState().rcuprotocol().read_error(global_rcu);
		}
		else {
			Cache::getInstance().getState().rcuread().read_error(global_rcu);
		}
		return GCFEvent::HANDLED;
	}

	// copy answer of board to internal var.
	uint8	boardVersion = (ack.result[resultOffset] & 0xF0) >> 4;	// calc version
	ack.result[resultOffset] &= 0x0F;								// remove from ack buffer
	uint32	rcuControlResult = 0;									// real mode of the RCU
	memcpy (&rcuControlResult, &ack.result[resultOffset], 3);		// place result in it

	// Note: we always apply the changes to the cache also when the I/O went wrong. This gives us
	// 	the possibility to show thses failures at the client site.
	Cache::getInstance().getBack().getRCUSettings()()((global_rcu)).setProtocolRaw(htonl(rcuControlResult));
	Cache::getInstance().getBack().getRCUSettings()()((global_rcu)).setVersion(boardVersion);
	Cache::getInstance().getFront().getRCUSettings()()((global_rcu)).setProtocolRaw(htonl(rcuControlResult));
	Cache::getInstance().getFront().getRCUSettings()()((global_rcu)).setVersion(boardVersion);

	// Finally compare the values with the values we have written.
	if (handlingWriteResult) {
		memcpy(RCUProtocolWrite::i2c_result_write + resultOffset, &rcuControlResult, 3);	// prepare internal buffer 
		if (memcmp(RCUProtocolWrite::i2c_result_write, ack.result, sizeof(RCUProtocolWrite::i2c_result_write)) == 0) {
			Cache::getInstance().getState().rcuprotocol().read_ack(global_rcu);
		} else {
			LOG_WARN("RCUResultRead::handleack: unexpected I2C write result response");
			Cache::getInstance().getState().rcuprotocol().read_error(global_rcu);
		}
	}
	else {	// only reading the values. Assume it is right.
		memcpy(RCUProtocolWrite::i2c_result_read + resultOffset, &rcuControlResult, 3);	// prepare internal buffer 
		if (memcmp(RCUProtocolWrite::i2c_result_read, ack.result, sizeof(RCUProtocolWrite::i2c_result_read)) == 0) {
			Cache::getInstance().getState().rcuread().read_ack(global_rcu);
		} else {
			LOG_WARN("RCUResultRead::handleack: unexpected I2C write result response");
			Cache::getInstance().getState().rcuread().read_error(global_rcu);
		}
	}

	return GCFEvent::HANDLED;
}
