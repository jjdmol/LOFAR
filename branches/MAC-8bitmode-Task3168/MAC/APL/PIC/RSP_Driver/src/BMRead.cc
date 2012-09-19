//#  BMRead.cc: implementation of the BMRead class
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
//#  $Id: BMRead.cc 18124 2011-05-29 19:54:09Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "BMRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

BMRead::BMRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&itsHdr, 0, sizeof(MEPHeader));
}

BMRead::~BMRead()
{
}

void BMRead::sendrequest()
{
  if ((( Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_maj * 10) +
         Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_min) < 74) {
    LOG_DEBUG_STR(formatString("BMRead:: Firmware on board[%d], has NO bitmode support", getBoardId()));
    setContinue(true); // continue with next action
  }
  else {
      EPAReadEvent bmread;
      bmread.hdr.set(MEPHeader::RSR_BEAMMODE_HDR,
                     MEPHeader::DST_RSP,
                     MEPHeader::READ );
    
      itsHdr = bmread.hdr;
      getBoardPort().send(bmread);
      LOG_DEBUG_STR("BMRead::sendrequest() done");
  }
}

void BMRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult BMRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RSR_BEAMMODE != event.signal)
  {
    LOG_WARN("BMRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  // unpack bm message
  EPARsrBeammodeEvent bm(event);

  LOG_DEBUG_STR(formatString("BM supported = %d", bm.beammode.bm_max));
  LOG_DEBUG_STR(formatString("BM selected  = %d", bm.beammode.bm_select));

  if (!bm.hdr.isValidAck(itsHdr))
  {
    LOG_ERROR("BMRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  LOG_DEBUG("handleack");

  LOG_DEBUG(formatString(">>>> BMRead(%s)", getBoardPort().getName().c_str()));
  
  Cache::getInstance().getBack().getBitModeInfo()()(getBoardId()).bm_select = bm.beammode.bm_select;
  Cache::getInstance().getBack().getBitModeInfo()()(getBoardId()).bm_max    = bm.beammode.bm_max;
  
  LOG_DEBUG_STR("BMRead::handleack() done");
  return GCFEvent::HANDLED;
}
