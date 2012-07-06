//#  BMWrite.cc: implementation of the BMWrite class
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
//#  $Id: BMWrite.cc 18124 2011-05-29 19:54:09Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "BMWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

BMWrite::BMWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, NR_BLPS_PER_RSPBOARD + 1)
{
  memset(&itsHdr, 0, sizeof(MEPHeader));
}

BMWrite::~BMWrite()
{
  /* TODO: delete event? */
}

void BMWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + getCurrentIndex();
  LOG_DEBUG(formatString(">>>> BMWrite(%s) global_blp=%d",
			 getBoardPort().getName().c_str(),
			 global_blp));

  // mark modified
  //Cache::getInstance().getState().ss().write_now(global_blp);
    
  // send subband select message
  EPARsrNofbeamEvent bm;
  bm.hdr.set(MEPHeader::RSR_NOFBEAM_HDR,
             0 == getCurrentIndex() ? MEPHeader::DST_RSP : 1 << (getCurrentIndex() - 1));
    
  bm.nofbeam.select = Cache::getInstance().getBack().getBitModeInfo()()(getBoardId()).select;
  
  itsHdr = bm.hdr;
  getBoardPort().send(bm);
}

void BMWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BMWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RSR_NOFBEAM != event.signal)
  {
    LOG_WARN("BMWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPARsrNofbeamEvent bm(event);

  //uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + getCurrentIndex();

  if (!bm.hdr.isValidAck(itsHdr))
  {
    //Cache::getInstance().getState().ss().write_error(global_blp);

    LOG_ERROR("BMWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  //Cache::getInstance().getState().ss().write_ack(global_blp);
  
  return GCFEvent::HANDLED;
}
