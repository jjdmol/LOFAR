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
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "RCUResultRead.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;


namespace LOFAR {
  namespace RSP {
    // construct expected i2c result
    uint8 i2c_result[] = { 0x00, // PROTOCOL_C_SEND_BLOCK OK
			   0xFF, // <<< replace with expected data >>>
			   0xFF, // <<< replace with expected data >>>
			   0xFF, // <<< replace with expected data >>>
			   0x00, // PROTOCOL_C_RECEIVE_BLOCK OK
			   0x00, // PROTOCOL_C_END OK
    };
  };
};

RCUResultRead::RCUResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) // *N_POL for X and Y
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUResultRead::~RCUResultRead()
{
  /* TODO: delete event? */
}

void RCUResultRead::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) + getCurrentIndex();

  // skip update if the RCU settings have not been modified
  if (RTC::RegisterState::NOT_MODIFIED == Cache::getInstance().getBack().getRCUSettings().getState().get(global_rcu))
  {
    setContinue(true);
    return;
  }

  // set appropriate header
  MEPHeader::FieldsType hdr;
  if (0 == global_rcu % 2) {
    hdr = MEPHeader::RCU_RESULTX_HDR;
  } else {
    hdr = MEPHeader::RCU_RESULTY_HDR;
  }

  EPAReadEvent rcuresult;
  rcuresult.hdr.set(hdr, 1 << (getCurrentIndex() / MEPHeader::N_POL), MEPHeader::READ, sizeof(i2c_result));
  
  m_hdr = rcuresult.hdr; // remember header to match with ack
  getBoardPort().send(rcuresult);
}

void RCUResultRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RCUResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RCU_RESULT != event.signal)
  {
    LOG_WARN("RCUResultRead::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPARcuResultEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("RCUResultRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) + getCurrentIndex();

  // reverse and copy control bytes into i2c_result
  RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
  uint32 control = htonl(rcucontrol.getRaw());
  memcpy(i2c_result + 1, &control, 3);

  if (0 == memcmp(i2c_result, ack.result, sizeof(i2c_result))) {
    uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) + getCurrentIndex();
    Cache::getInstance().getBack().getRCUSettings().getState().applied(global_rcu);
  } else {
    LOG_ERROR("RCUResultRead::handleack: unexpected I2C result response");
  }

  return GCFEvent::HANDLED;
}
