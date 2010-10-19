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
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

VersionsRead::~VersionsRead()
{
  /* TODO: delete event? */
}

void VersionsRead::sendrequest()
{
  // send version read request
  EPAReadEvent versionread;
  versionread.hdr.set(MEPHeader::RSR_VERSION_HDR);

  m_hdr = versionread.hdr;
  getBoardPort().send(versionread);
}

void VersionsRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult VersionsRead::handleack(GCFEvent& event, GCFPortInterface& port)
{
  if (EPA_RSR_VERSION != event.signal)
  {
    LOG_WARN("VersionsRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPARsrVersionEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("VersionsRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  LOG_DEBUG(formatString("Firmware versions on board '%s' are [rsp:%d.%d, bp:%d.%d, ap:%d.%d",
			 port.getName().c_str(),
			 ack.rsp_version   >> 4, ack.rsp_version   & 0xF,
			 ack.bp_version    >> 4, ack.bp_version    & 0xF,
			 ack.ap_version    >> 4, ack.ap_version    & 0xF));
  
  Cache::getInstance().getBack().getVersions().rsp()(getBoardId()) = ack.rsp_version;
  Cache::getInstance().getBack().getVersions().bp()(getBoardId())  = ack.bp_version;  
  Cache::getInstance().getBack().getVersions().ap()(getBoardId())  = ack.ap_version;  

  return GCFEvent::HANDLED;
}
