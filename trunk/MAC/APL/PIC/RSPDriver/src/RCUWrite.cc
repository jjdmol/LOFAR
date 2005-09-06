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

#include "RCUWrite.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>
#include <string.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

RCUWrite::RCUWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i))
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

RCUWrite::~RCUWrite()
{
  /* TODO: delete event? */
}

void RCUWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();
  EPARcuSettingsEvent rcusettings;

  rcusettings.hdr.set(MEPHeader::RCU_SETTINGS_HDR, getCurrentBLP());

  RCUSettings::RCURegisterType& x = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2));
  RCUSettings::RCURegisterType& y = Cache::getInstance().getBack().getRCUSettings()()((global_blp * 2) + 1);

  memcpy(&rcusettings.x, &x, sizeof(uint8));
  memcpy(&rcusettings.y, &y, sizeof(uint8));

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

  return GCFEvent::HANDLED;
}
