//#  VersionsRead.cc: implementation of the VersionsRead class
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

#include "VersionsRead.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

VersionsRead::VersionsRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
}

VersionsRead::~VersionsRead()
{
  /* TODO: delete event? */
}

void VersionsRead::sendrequest()
{
  // send read status request to check status of the write
  EPAFwversionReadEvent versionread;
  MEP_FWVERSION(versionread.hdr, MEPHeader::READ);

  getBoardPort().send(versionread);
}

void VersionsRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult VersionsRead::handleack(GCFEvent& event, GCFPortInterface& port)
{
  EPAFwversionEvent ack(event);

  LOG_DEBUG(formatString("Firmware versions on board '%s' are [rsp:%d.%d, bp:%d.%d, ap[0]:%d.%d, ap[1]:%d.%d, ap[2]:%d.%d, ap[3]:%d.%d",
			port.getName().c_str(),
			ack.rsp_version   >> 4, ack.rsp_version   & 0xF,
			ack.bp_version    >> 4, ack.bp_version    & 0xF,
			ack.ap_version[0] >> 4, ack.ap_version[0] & 0xF,
			ack.ap_version[1] >> 4, ack.ap_version[1] & 0xF,
			ack.ap_version[2] >> 4, ack.ap_version[2] & 0xF,
			ack.ap_version[3] >> 4, ack.ap_version[3] & 0xF));
  
  Cache::getInstance().getBack().getVersions().rsp()(getBoardId()) = ack.rsp_version;
  Cache::getInstance().getBack().getVersions().bp()(getBoardId())  = ack.bp_version;  
  for (int i = 0; i < EPA_Protocol::N_AP; i++)
  {
    Cache::getInstance().getBack().getVersions().ap()((getBoardId() * EPA_Protocol::N_AP) + i)
      = ack.ap_version[i];
  }

  return GCFEvent::HANDLED;
}
