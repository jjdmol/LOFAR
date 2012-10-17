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
  : SyncAction(board_port, board_id, 1)
{
  memset(&itsHdr, 0, sizeof(MEPHeader));
}

BMWrite::~BMWrite()
{
  /* TODO: delete event? */
}

void BMWrite::sendrequest()
{
  if ((( Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_maj * 10) +
         Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_min) < 74) {
    LOG_DEBUG_STR(formatString("BMWrite:: Firmware on board[%d], has NO bitmode support", getBoardId()));
    setContinue(true); // continue with next action
  }
  else {
      
      LOG_DEBUG(formatString(">>>> BMWrite(%s) boardId=%d",
    			 getBoardPort().getName().c_str(),
    			 getBoardId()));
      
      // skip update if the neither of the RCU's settings have been modified
      if (RTC::RegisterState::WRITE != Cache::getInstance().getState().bmState().get(getBoardId())) {
        Cache::getInstance().getState().bmState().unmodified(getBoardId());
        setContinue(true);
    
        return;
      }
        
      // send subband select message
      EPARsrBeammodeEvent bm;
      bm.hdr.set(MEPHeader::RSR_BEAMMODE_HDR,
                 MEPHeader::DST_ALL);
        
      bm.beammode.bm_select = Cache::getInstance().getBack().getBitModeInfo()()(getBoardId()).bm_select;
      
      itsHdr = bm.hdr;
LOG_INFO_STR(bm);
      getBoardPort().send(bm);
  }
}

void BMWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BMWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("BMWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent bm(event);

  if (!bm.hdr.isValidAck(itsHdr))
  {
    Cache::getInstance().getState().bmState().write_error(getBoardId());
    LOG_ERROR("BMWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().bmState().write_ack(getBoardId());
  
  return GCFEvent::HANDLED;
}
