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

void RCUResultRead::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  // skip update if the RCU settings have not been applied yet
  if (RTC::RegisterState::READ != Cache::getInstance().getState().rcuprotocol().get(global_rcu)) {
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
  rcuresult.hdr.set(hdr, 1 << (getCurrentIndex() / MEPHeader::N_POL), MEPHeader::READ, sizeof(RCUProtocolWrite::i2c_result));
  
  m_hdr = rcuresult.hdr; // remember header to match with ack
  getBoardPort().send(rcuresult);
}

void RCUResultRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RCUResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RCU_RESULT != event.signal) {
    LOG_WARN("RCUResultRead::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPARcuResultEvent ack(event);

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  if (!ack.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("RCUResultRead::handleack: invalid ack");
    Cache::getInstance().getState().rcuprotocol().read_error(global_rcu);
    return GCFEvent::HANDLED;
  }
  
 
  
  
  // reverse and copy control bytes into i2c_result
  RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
  
  //LOG_INFO_STR(formatString("rcucontrol 1= %08X", rcucontrol.getRaw()));
  
  // first add RCU version to cache  //PD
  rcucontrol.setVersion((ack.result[1] & 0xF0) >> 4);
  //LOG_INFO_STR(formatString("ack.result(%d)= %u %u %u",global_rcu, ack.result[1], ack.result[2], ack.result[3] ));
  //LOG_INFO_STR(formatString("rcucontrol 2= %08X", rcucontrol.getRaw()));
  
  Cache::getInstance().getBack().getRCUSettings()()((global_rcu)) = rcucontrol;
  Cache::getInstance().getFront().getRCUSettings()()((global_rcu)) = rcucontrol;
   
  uint32 control = htonl(rcucontrol.getRaw());
  //LOG_INFO_STR(formatString("control 1= %08X", control));
  memcpy(RCUProtocolWrite::i2c_result + 1, &control, 3);

  if (0 == memcmp(RCUProtocolWrite::i2c_result, ack.result, sizeof(RCUProtocolWrite::i2c_result))) {
    Cache::getInstance().getState().rcuprotocol().read_ack(global_rcu);
  } else {
    LOG_WARN("RCUResultRead::handleack: unexpected I2C result response");
    Cache::getInstance().getState().rcuprotocol().read_error(global_rcu);
  }

  return GCFEvent::HANDLED;
}
