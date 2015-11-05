//#  TimestampWrite.cc: implementation of the TimestampWrite class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "TimestampWrite.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;
using namespace blitz;

TimestampWrite::TimestampWrite(GCFPortInterface& board_port, int board_id, const Scheduler& scheduler)
  : SyncAction(board_port, board_id, 1),  m_scheduler(scheduler)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TimestampWrite::~TimestampWrite()
{
  /* TODO: delete event? */
}

void TimestampWrite::sendrequest()
{
  EPARsrTimestampEvent ts;
    
  // send timestamp to all FPGA's (ALL BLP's and RSP)
  // RSP uses it on CEP output
  // BLP's use it on TBB data output via the backplane
  ts.hdr.set(MEPHeader::RSR_TIMESTAMP_HDR, MEPHeader::DST_ALL, MEPHeader::WRITE);
  ts.timestamp = m_scheduler.getCurrentTime().sec() + 1;
  
  m_hdr = ts.hdr;
  getBoardPort().send(ts);
}

void TimestampWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TimestampWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("TimestampWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("TimestampWrite::handleack: invalid ack");
    Cache::getInstance().getState().ts().write_error(getBoardId());
    return GCFEvent::NOT_HANDLED;
  }

  // change state to indicate that it has been applied in the hardware
  Cache::getInstance().getState().ts().write_ack(getBoardId());

  return GCFEvent::HANDLED;
}
