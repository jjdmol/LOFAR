//#  SSWrite.cc: implementation of the SSWrite class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SSWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SSWrite::SSWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

SSWrite::~SSWrite()
{
  /* TODO: delete event? */
}

void SSWrite::sendrequest()
{
  uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();
  LOG_DEBUG(formatString(">>>> SSWrite(%s) global_blp=%d",
			 getBoardPort().getName().c_str(),
			 global_blp));
    
  // send subband select message
  EPASsSelectEvent ss;
  ss.hdr.set(MEPHeader::SS_SELECT_HDR, 1 << getCurrentIndex());
    
  // create array to contain the subband selection
  Array<uint16, 2> subbands((uint16*)&ss.subbands,
			    shape(MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS, MEPHeader::N_POL),
			    neverDeleteData);
    
  // copy the actual values from the cache
  subbands(Range::all(), 0) = Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2,     Range::all()); // x
  subbands(Range::all(), 1) = Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2 + 1, Range::all()); // y

  m_hdr = ss.hdr;
  getBoardPort().send(ss);
}

void SSWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult SSWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("SSWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("SSWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  return GCFEvent::HANDLED;
}
