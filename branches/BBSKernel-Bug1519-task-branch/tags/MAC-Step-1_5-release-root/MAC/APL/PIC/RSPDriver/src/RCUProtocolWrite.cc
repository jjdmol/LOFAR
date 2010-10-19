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

    uint8 RCUProtocolWrite::i2c_protocol[RCUProtocolWrite::PROTOCOL_SIZE] 
    = { 0x0F, // PROTOCOL_C_SEND_BLOCK
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

    uint8 RCUProtocolWrite::i2c_result[RCUProtocolWrite::RESULT_SIZE] 
    = { 0x00, // PROTOCOL_C_SEND_BLOCK OK
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

void RCUProtocolWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + (getCurrentIndex() / N_WRITES);

  // skip update if the RCU settings have not been modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().rcuprotocol().get(global_rcu)) {
    Cache::getInstance().getState().rcuprotocol().unmodified(global_rcu);
    setContinue(true);
    return;
  }

  switch (getCurrentIndex() % N_WRITES) {
    
  case 0:
    {
      // reverse and copy control bytes into i2c_protocol
      RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
      uint32 control = htonl(rcucontrol.getRaw());
      memcpy(i2c_protocol+3, &control, 3);

      // set appropriate header
      MEPHeader::FieldsType hdr;
      if (0 == global_rcu % MEPHeader::N_POL) {
	hdr = MEPHeader::RCU_PROTOCOLX_HDR;
      } else {
	hdr = MEPHeader::RCU_PROTOCOLY_HDR;
      }

      EPARcuProtocolEvent rcuprotocol;
      rcuprotocol.hdr.set(hdr, 1 << (getCurrentIndex() / (MEPHeader::N_POL * N_WRITES)), MEPHeader::WRITE, sizeof(i2c_protocol));
      rcuprotocol.protocol.setBuffer(i2c_protocol, sizeof(i2c_protocol));
  
      m_hdr = rcuprotocol.hdr; // remember header to match with ack
      getBoardPort().send(rcuprotocol);
    }
    break;

  case 1:
    {
      EPAWriteEvent rcuresultwrite;

      // set appropriate header
      uint8 regid = 0;
      if (0 == (global_rcu % MEPHeader::N_POL)) {
	regid = MEPHeader::RCU_RESULTX;
      } else {
	regid = MEPHeader::RCU_RESULTY;
      }

      rcuresultwrite.hdr.set(MEPHeader::WRITE, 1 << (getCurrentIndex() / (MEPHeader::N_POL * N_WRITES)),
			     MEPHeader::RCU, regid, sizeof(i2c_result), 0);
      uint8 clear[RESULT_SIZE];
      memset(clear, 0xAA, RESULT_SIZE); // clear result
      rcuresultwrite.payload.setBuffer(clear, RESULT_SIZE);

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
  
  EPAWriteackEvent ack(event);

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + (getCurrentIndex() / N_WRITES);

  if (!ack.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("RCUProtocolWrite::handleack: invalid ack");
    Cache::getInstance().getState().rcuprotocol().write_error(global_rcu);
    return GCFEvent::NOT_HANDLED;
  }

  if (1 == (getCurrentIndex() % N_WRITES)) {

    // Mark modification as applied when write of RCU result register has completed

    Cache::getInstance().getState().rcuprotocol().read_schedule(global_rcu);

  }
  
  return GCFEvent::HANDLED;
}
