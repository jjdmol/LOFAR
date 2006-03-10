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
    // construct i2c sequence
    static uint8 i2c_protocol[] = { 0x0F, // PROTOCOL_C_SEND_BLOCK
				    0x01, // I2C address for RCU
				    0x03, // size
				    0xFF, // <<< replace with data >>>
				    0xFF, // <<< replace with data >>>
				    0xFF, // <<< replace with data >>>
				    0x10, // PROTOCOL_C_RECEIVE_BLOCK
				    0x01, // I2C adress for RCU
				    0x03, // requested size
				    0x13, // PROTOCOL_C_END
    };
  };
};

RCUProtocolWrite::RCUProtocolWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) // *N_POL for X and Y
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUProtocolWrite::~RCUProtocolWrite()
{
  /* TODO: delete event? */
}

void RCUProtocolWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) + getCurrentIndex();

  // skip update if the RCU settings have not been modified
  if (RTC::RegisterState::MODIFIED != Cache::getInstance().getBack().getRCUSettings().getState().get(global_rcu))
  {
    setContinue(true);
    return;
  }

  // reverse and copy control bytes into i2c_protocol
  RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
  uint32 control = htonl(rcucontrol.getRaw());
  memcpy(i2c_protocol+3, &control, 3);

  // set appropriate header
  MEPHeader::FieldsType hdr;
  if (0 == global_rcu % 2) {
    hdr = MEPHeader::RCU_PROTOCOLX_HDR;
  } else {
    hdr = MEPHeader::RCU_PROTOCOLY_HDR;
  }

  EPARcuProtocolEvent rcuprotocol;
  rcuprotocol.hdr.set(hdr, 1 << (getCurrentIndex() / MEPHeader::N_POL), MEPHeader::WRITE, sizeof(i2c_protocol));
  rcuprotocol.protocol.setBuffer(i2c_protocol, sizeof(i2c_protocol));
  //  memcpy(rcuprotocol.protocol, i2c_protocol, sizeof(i2c_protocol));
  
  m_hdr = rcuprotocol.hdr; // remember header to match with ack
  getBoardPort().send(rcuprotocol);
}

void RCUProtocolWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RCUProtocolWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("RCUProtocolWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("RCUProtocolWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  // Mark as register modification as applied
  // Still needs to be confirmed by RCUResultRead
  uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL) + getCurrentIndex();
  Cache::getInstance().getBack().getRCUSettings().getState().applied(global_rcu);

  return GCFEvent::HANDLED;
}
