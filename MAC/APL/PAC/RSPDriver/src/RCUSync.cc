//#  RCUSync.cc: implementation of the RCUSync class
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

#include "RCUSync.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

RCUSync::RCUSync(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, N_BLP / 2)
{
}

RCUSync::~RCUSync()
{
  /* TODO: delete event? */
}

void RCUSync::sendrequest(int iteration)
{
  uint8 blp = (getBoardId() * N_BLP) + iteration * 2;

  EPARcusettingsEvent rcusettings;
  MEP_RCUSETTINGS(rcusettings.hdr, MEPHeader::WRITE, 0);
  rcusettings.hdr.m_fields.addr.dstid = blp;

  RCUSettings x = Cache::getInstance().getBack().getRCUSettings(blp);
  RCUSettings y = Cache::getInstance().getBack().getRCUSettings(blp + 1);
  memcpy(&rcusettings.x, &x, sizeof(uint8));
  memcpy(&rcusettings.y, &y, sizeof(uint8));

  getBoardPort().send(rcusettings);
}

void RCUSync::sendrequest_status()
{
  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);
  
  getBoardPort().send(rspstatus);
}

GCFEvent::TResult RCUSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent ack(event);

  return GCFEvent::HANDLED;
}
