//#  RCUProtocolWrite.cc: implementation of the RCUProtocolWrite class
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
#include "RCUProtocolWrite.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

namespace LOFAR {
  namespace RSP {

    const int RCUProtocolWrite::PROTOCOL_WRITE_SIZE;
    const int RCUProtocolWrite::PROTOCOL_READ_SIZE;
    const int RCUProtocolWrite::RESULT_WRITE_SIZE;
    const int RCUProtocolWrite::RESULT_READ_SIZE;

    uint8 RCUProtocolWrite::i2c_protocol_write[] = { 
		0x0F, // PROTOCOL_C_SEND_BLOCK
		0x01, // I2C address for RCU
		0x03, // size
		0xAA, // <<< replace with data >>>
		0xAA, // <<< replace with data >>>
		0xAA, // <<< replace with data >>>
		0x10, // PROTOCOL_C_RECEIVE_BLOCK
		0x01, // I2C adress for RCU
		0x03, // requested size
		0x13, // PROTOCOL_C_END
    };

    uint8 RCUProtocolWrite::i2c_protocol_read[] = { 
		0x10, // PROTOCOL_C_RECEIVE_BLOCK
		0x01, // I2C adress for RCU
		0x03, // requested size
		0x13, // PROTOCOL_C_END
    };

    uint8 RCUProtocolWrite::i2c_result_write[] = { 
		0x00, // PROTOCOL_C_SEND_BLOCK OK
		0xAA, // <<< replace with expected data >>>
		0xAA, // <<< replace with expected data >>>
		0xAA, // <<< replace with expected data >>>
		0x00, // PROTOCOL_C_RECEIVE_BLOCK OK
		0x00, // PROTOCOL_C_END OK
    };

    uint8 RCUProtocolWrite::i2c_result_read[] = { 
		0xAA, // <<< replace with expected data >>>
		0xAA, // <<< replace with expected data >>>
		0xAA, // <<< replace with expected data >>>
		0x00, // PROTOCOL_C_RECEIVE_BLOCK OK
		0x00, // PROTOCOL_C_END OK
    };
  };
};

#define N_WRITES 2 // 2 writes, one for protocol register, one to clear results register

RCUProtocolWrite::RCUProtocolWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrRcusPerBoard() * N_WRITES) // *N_POL for X and Y
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUProtocolWrite::~RCUProtocolWrite()
{
  /* TODO: delete event? */
}

//
// sendrequest()
//
// Note: we come here if the user has requested a rcu write OR rcu read command
//
void RCUProtocolWrite::sendrequest()
{
	uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + (getCurrentIndex() / N_WRITES);
	bool	writeCmdRequested(true);		// assume setting the rcumode

	// should we write the RCU?
	if (Cache::getInstance().getState().rcuprotocol().get(global_rcu) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().rcuprotocol().unmodified(global_rcu);
		writeCmdRequested = false;			// not setting the rcumode, maybe reading?
	}

	// should we read the RCU?
	if (Cache::getInstance().getState().rcuread().get(global_rcu) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().rcuread().unmodified(global_rcu);
		if (!writeCmdRequested) {	// both commands not needed, then we are finished.
			setContinue(true);
			return;
		}
	}

	// We need two writes per RCU, first we write a long i2c stream to the rcu. Second we clear the
	// result registers of the i2c.
	switch (getCurrentIndex() % N_WRITES) {
	case 0: {
			// set appropriate header
			MEPHeader::FieldsType hdr;
			if (0 == global_rcu % MEPHeader::N_POL) {
				hdr = MEPHeader::RCU_PROTOCOLX_HDR;
			} else {
				hdr = MEPHeader::RCU_PROTOCOLY_HDR;
			}

			if (writeCmdRequested) {
				// reverse and copy control bytes into i2c_protocol_write
				RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
				uint32 control = htonl(rcucontrol.getRaw());
				memcpy(i2c_protocol_write+3, &control, 3);

				EPARcuProtocolEvent rcuprotocol;
				rcuprotocol.hdr.set(hdr, 1 << (getCurrentIndex() / (MEPHeader::N_POL * N_WRITES)), MEPHeader::WRITE, sizeof(i2c_protocol_write));
				rcuprotocol.protocol.setBuffer(i2c_protocol_write, sizeof(i2c_protocol_write));

				m_hdr = rcuprotocol.hdr; // remember header to match with ack
				getBoardPort().send(rcuprotocol);
				break;
			}
			// user wants to read the RCUs
			EPARcuProtocolEvent rcuprotocol;
			rcuprotocol.hdr.set(hdr, 1 << (getCurrentIndex() / (MEPHeader::N_POL * N_WRITES)), MEPHeader::WRITE, sizeof(i2c_protocol_read));
			rcuprotocol.protocol.setBuffer(i2c_protocol_read, sizeof(i2c_protocol_read));

			m_hdr = rcuprotocol.hdr; // remember header to match with ack
			getBoardPort().send(rcuprotocol);
		}
		break;

	case 1: {
			EPAWriteEvent rcuresultwrite;
			// set appropriate header
			uint8 regid = 0;
			if (0 == (global_rcu % MEPHeader::N_POL)) {
				regid = MEPHeader::RCU_RESULTX;
			} else {
				regid = MEPHeader::RCU_RESULTY;
			}

			int		resultSize = writeCmdRequested ? RESULT_WRITE_SIZE : RESULT_READ_SIZE;
			rcuresultwrite.hdr.set(MEPHeader::WRITE, 1 << (getCurrentIndex() / (MEPHeader::N_POL * N_WRITES)),
									MEPHeader::RCU, regid, resultSize, 0);
			uint8 clear[RESULT_WRITE_SIZE];
			memset(clear, 0xAA, RESULT_WRITE_SIZE); // clear result
			rcuresultwrite.payload.setBuffer(clear, resultSize);

			m_hdr = rcuresultwrite.hdr; // remember header to match with ack
			getBoardPort().send(rcuresultwrite);
		}
		break;
	}
}

void RCUProtocolWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult RCUProtocolWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal) {
		LOG_WARN("RCUProtocolWrite::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + (getCurrentIndex() / N_WRITES);

	EPAWriteackEvent ack(event);
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("RCUProtocolWrite::handleack: invalid ack");
		if (m_hdr.m_fields.payload_length == RESULT_WRITE_SIZE) {
			Cache::getInstance().getState().rcuprotocol().write_error(global_rcu);
		}
		else {
			Cache::getInstance().getState().rcuread().write_error(global_rcu);
		}
		return GCFEvent::NOT_HANDLED;
	}

	if ((getCurrentIndex() % N_WRITES) == 1) {
		// Mark modification as applied when write of RCU result register has completed
		if (m_hdr.m_fields.payload_length == RESULT_WRITE_SIZE) {
			Cache::getInstance().getState().rcuprotocol().schedule_wait1read(global_rcu);
		}
		else {
			Cache::getInstance().getState().rcuread().schedule_wait1read(global_rcu);
		}
	}

	return GCFEvent::HANDLED;
}
