//#  RCURead.cc: implementation of the RCURead class
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
#include "RCURead.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

RCURead::RCURead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCURead::~RCURead()
{
}

void RCURead::sendrequest()
{
  EPAReadEvent rcusettingsread;
  rcusettingsread.hdr.set(MEPHeader::RCU_SETTINGS_HDR,
			  1 << getCurrentIndex(),
			  MEPHeader::READ);

  m_hdr = rcusettingsread.hdr;
  getBoardPort().send(rcusettingsread);
}

void RCURead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult RCURead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RCU_SETTINGS != event.signal) {
    LOG_WARN("RCURead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPARcuSettingsEvent rcusettings(event);

  if (!rcusettings.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("RCURead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();

  // This needs to be replaced by I2C sequences
  RCUSettings::Control& x = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2));
  RCUSettings::Control& y = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2) + 1);

  if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i)) {
    EPA_Protocol::RCUHandler cachedvalue = { x.getDelay(), 0, y.getDelay(), 0 };
    if (memcmp(&cachedvalue, &rcusettings.ap, sizeof(EPA_Protocol::RCUHandler))) {
      LOG_WARN("LOOPBACK CHECK FAILED: RCURead mismatch ");
    }
  }
  else {
    x.setDelay(rcusettings.ap.input_delay_x);
    y.setDelay(rcusettings.ap.input_delay_y);
  }

  return GCFEvent::HANDLED;
}
