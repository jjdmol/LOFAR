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
#include "Cache.h"

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

VersionsSync::VersionsSync(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
}

VersionsSync::~VersionsSync()
{
  /* TODO: delete event? */
}

void VersionsSync::sendrequest(int /*iteration*/)
{
  // send read status request to check status of the write
  EPAFwversionReadEvent versionread;
  MEP_FWVERSION(versionread.hdr, MEPHeader::READ);

  getBoardPort().send(versionread);
}

void VersionsSync::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult VersionsSync::handleack(GCFEvent& event, GCFPortInterface& port)
{
  EPAFwversionEvent ack(event);

  LOG_INFO(formatString("Firmware version on board '%s'=%d",
			port.getName().c_str(), ack.version));

  Cache::getInstance().getBack().getVersions()()(getBoardId()) = ack.version;

  return GCFEvent::HANDLED;
}
