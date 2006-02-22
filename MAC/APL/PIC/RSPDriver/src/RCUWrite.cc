//#  RCUWrite.cc: implementation of the RCUWrite class
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
#include "RCUWrite.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

RCUWrite::RCUWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUWrite::~RCUWrite()
{
  /* TODO: delete event? */
}

void RCUWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();

  // skip update if the neither of the RCU's settings have been modified
  if (RTC::RegisterState::NOT_MODIFIED == Cache::getInstance().getBack().getRCUSettings().getState().get(global_blp * 2)
      && RTC::RegisterState::NOT_MODIFIED == Cache::getInstance().getBack().getRCUSettings().getState().get(global_blp * 2 + 1)) {
    setContinue(true);
    return;
  }

  // This needs to be replaced by I2C sequences
  RCUSettings::Control& x = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2));
  RCUSettings::Control& y = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2) + 1);

  LOG_INFO(formatString("%d.X control=0x%08x", global_blp, x.getRaw()));
  LOG_INFO(formatString("%d.Y control=0x%08x", global_blp, y.getRaw()));

  EPARcuSettingsEvent rcusettings;
  rcusettings.hdr.set(MEPHeader::RCU_SETTINGS_HDR, 1 << getCurrentIndex()); // also sets payload_length
  rcusettings.ap = EPA_Protocol::RCUHandler();
  rcusettings.ap.spec_inv_x    = x.getSpecinv();
  rcusettings.ap.spec_inv_y    = y.getSpecinv();
  rcusettings.ap.input_delay_x = x.getDelay();
  rcusettings.ap.input_delay_y = y.getDelay();

  m_hdr = rcusettings.hdr;
  getBoardPort().send(rcusettings);
}

void RCUWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RCUWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("RCUWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("RCUWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();
  Cache::getInstance().getBack().getRCUSettings().getState().applied(global_blp);

  return GCFEvent::HANDLED;
}
