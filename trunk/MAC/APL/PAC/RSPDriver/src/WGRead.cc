//#  WGRead.cc: implementation of the WGRead class
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

#include "WGRead.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

WGRead::WGRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * 2)
{
}

WGRead::~WGRead()
{
  /* TODO: delete event? */
}

void WGRead::sendrequest()
{
  EPAReadEvent wgsettingsread;

  if (0 == getCurrentBLP() % MEPHeader::N_POL)
  {
    wgsettingsread.hdr.set(MEPHeader::WG_XSETTINGS_HDR,
			   getCurrentBLP() / 2,
			   MEPHeader::READ);
  }
  else
  {
    wgsettingsread.hdr.set(MEPHeader::WG_YSETTINGS_HDR,
			   getCurrentBLP() / 2,
			   MEPHeader::READ);
  }
  
  getBoardPort().send(wgsettingsread);
}

void WGRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult WGRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (event.signal != EPA_WG_SETTINGS) return GCFEvent::HANDLED;
  
  uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  EPAWgSettingsEvent wgsettings(event);

  WGSettings& w = Cache::getInstance().getBack().getWGSettings();

  memcpy(&(w()(global_rcu)), &wgsettings.freq, sizeof(WGSettings::WGRegisterType));

  return GCFEvent::HANDLED;
}
