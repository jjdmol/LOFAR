//#  SSRead.cc: implementation of the SSRead class
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

#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"
#include "SSRead.h"
#include "Cache.h"

#include <PSAccess.h>
#include <blitz/array.h>

#define N_RETRIES 3

using namespace RSP;
using namespace LOFAR;
using namespace blitz;
using namespace RTC;

SSRead::SSRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i))
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

SSRead::~SSRead()
{
}

void SSRead::sendrequest()
{
  EPAReadEvent ssread;
  ssread.hdr.set(MEPHeader::SS_SELECT_HDR, getCurrentBLP(),
		 MEPHeader::READ);

  m_hdr = ssread.hdr;
  getBoardPort().send(ssread);
}

void SSRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult SSRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_SS_SELECT != event.signal)
  {
    LOG_WARN("SSRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  // unpack ss message
  EPASsSelectEvent ss(event);

  if (!ss.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("SSRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  uint8 global_blp = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  LOG_DEBUG("handleack");

  LOG_DEBUG(formatString(">>>> SSRead(%s) global_blp=%d",
			 getBoardPort().getName().c_str(), global_blp));
  
  // create array point to data in the response event
  Array<uint16, 2> subbands((uint16*)&ss.ch,
			    shape(MEPHeader::N_BEAMLETS, MEPHeader::N_POL),
			    neverDeleteData);

  if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
  {
    subbands(Range::all(), 0) -= Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2,     Range::all());
    subbands(Range::all(), 1) -= Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2 + 1, Range::all());
    uint16 ssum = sum(subbands);

    if (0 != ssum)
    {
      LOG_WARN(formatString("LOOPBACK CHECK FAILED: SSRead mismatch (blp=%d, error=%d)",
			    global_blp, ssum));
    }
  }
  else
  {
    // copy into the cache
    Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2, Range::all())
      = subbands(Range::all(), 0); // x
    Cache::getInstance().getBack().getSubbandSelection()()(global_blp * 2 + 1, Range::all())
      = subbands(Range::all(), 1); // y
  }
  return GCFEvent::HANDLED;
}
