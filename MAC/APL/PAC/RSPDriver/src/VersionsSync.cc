//#  VersionsSync.cc: implementation of the VersionsSync class
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

#include "VersionsSync.h"
#include "EPA_Protocol.ph"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

VersionsSync::VersionsSync(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id)
{
}

VersionsSync::~VersionsSync()
{
  /* TODO: delete event? */
}

void VersionsSync::sendrequest(uint8 /*blp*/)
{
}

void VersionsSync::sendrequest_status()
{
  // send read status request to check status of the write
  EPARspstatusEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);
  
  // clear from first field onwards
  memset(&rspstatus.board, 0, MEPHeader::RSPSTATUS_SIZE);

#if 0
  // on the read request don't send the data
  rspstatus.length -= RSPSTATUS_SIZE;
#endif

  getBoardPort().send(rspstatus);
}

GCFEvent::TResult VersionsSync::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  EPARspstatusEvent ack(event);

  return GCFEvent::HANDLED;
}
