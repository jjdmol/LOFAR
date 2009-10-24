//#  TBBBandselWrite.cc: implementation of the TBBBandselWrite class
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
#include "TBBBandselWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

TBBBandselWrite::TBBBandselWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrRcusPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TBBBandselWrite::~TBBBandselWrite()
{
  /* TODO: delete event? */
}

void TBBBandselWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  // skip update if not modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().tbbbandsel().get(global_rcu)) {
    Cache::getInstance().getState().tbbbandsel().unmodified(global_rcu);
    setContinue(true);
    return;
  }
   
  LOG_DEBUG(formatString(">>>> TBBBandselWrite(%s) global_rcu=%d",
			 getBoardPort().getName().c_str(),
			 global_rcu));

  // send TBB bandsel
  EPATbbBandselEvent tbbbandsel;
  tbbbandsel.hdr.set(MEPHeader::WRITE,
		      1 << (getCurrentIndex() / MEPHeader::N_POL),
		      MEPHeader::TBB,
		      MEPHeader::TBB_BANDSELX + (getCurrentIndex() % MEPHeader::N_POL),
		      MEPHeader::TBB_BANDSEL_SIZE);
		      
  tbbbandsel.bandsel = Cache::getInstance().getBack().getTBBSettings()()(global_rcu);

  LOG_INFO_STR("TBBBandselWrite (rcu=" << (int)global_rcu << " = " << tbbbandsel.bandsel);

  m_hdr = tbbbandsel.hdr;
  getBoardPort().send(tbbbandsel);
}

void TBBBandselWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TBBBandselWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal) {
    LOG_WARN("TBBBandselWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  if (!ack.hdr.isValidAck(m_hdr)) {
    Cache::getInstance().getState().tbbbandsel().write_error(global_rcu);

    LOG_ERROR("TBBBandselWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().tbbbandsel().write_ack(global_rcu);
  
  return GCFEvent::HANDLED;
}
