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

#include "RCURead.h"
#include "EPA_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>
#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;

RCURead::RCURead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i))
{
}

RCURead::~RCURead()
{
}

void RCURead::sendrequest()
{
  EPARcuSettingsEvent rcusettingsread;
  rcusettingsread.hdr.set(MEPHeader::RCU_SETTINGS_HDR,
			  MEPHeader::READ,
			  getCurrentBLP());

  getBoardPort().send(rcusettingsread);
}

void RCURead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult RCURead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (event.signal != EPA_RCU_SETTINGS) return GCFEvent::HANDLED;
  
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP() * 2;

  EPARcuSettingsEvent rcusettings(event);

  RCUSettings::RCURegisterType& x = Cache::getInstance().getBack().getRCUSettings()()(global_blp);
  RCUSettings::RCURegisterType& y = Cache::getInstance().getBack().getRCUSettings()()(global_blp + 1);

  memcpy(&x, &rcusettings.x, sizeof(uint8));
  memcpy(&y, &rcusettings.y, sizeof(uint8));

  return GCFEvent::HANDLED;
}
