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

#include "RCUWrite.h"
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

RCUWrite::RCUWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i))
{
}

RCUWrite::~RCUWrite()
{
  /* TODO: delete event? */
}

void RCUWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  EPARcusettingsEvent rcusettings;
  MEP_RCUSETTINGS(rcusettings.hdr, MEPHeader::WRITE, getCurrentBLP());

  RCUSettings::RCURegisterType& x = Cache::getInstance().getBack().getRCUSettings()()(global_blp);
  RCUSettings::RCURegisterType& y = Cache::getInstance().getBack().getRCUSettings()()(global_blp + 1);

#ifdef TOGGLE_LEDS
  x.filter_0 = Cache::getInstance().ledstatus();
  x.filter_1 = !Cache::getInstance().ledstatus();
#endif

  memcpy(&rcusettings.x, &x, sizeof(uint8));
  memcpy(&rcusettings.y, &y, sizeof(uint8));

  getBoardPort().send(rcusettings);
}

void RCUWrite::sendrequest_status()
{
#if WRITE_ACK_VERREAD
  // send version read request
  EPAFwversionReadEvent versionread;
  MEP_FWVERSION(versionread.hdr, MEPHeader::READ);

  getBoardPort().send(versionread);
#else
  // send read status request to check status of the write
  EPARspstatusReadEvent rspstatus;
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READ);
  
  getBoardPort().send(rspstatus);
#endif
}

GCFEvent::TResult RCUWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
#if WRITE_ACK_VERREAD
  EPAFwversionEvent ack(event);
#else
  EPARspstatusEvent ack(event);
#endif

  return GCFEvent::HANDLED;
}
