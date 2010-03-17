//#  BSWrite.cc: implementation of the BSWrite class
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
#include "BSWrite.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;
using namespace blitz;

BSWrite::BSWrite(GCFPortInterface& board_port, int board_id, int blp, const Scheduler& scheduler)
  : SyncAction(board_port, board_id, 1), m_blp(blp), m_scheduler(scheduler)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));

  // this action should be performed at initialisation
  doAtInit();
}

BSWrite::~BSWrite()
{
  /* TODO: delete event? */
}

void BSWrite::sendrequest()
{
  // skip update if the neither of the RCU's settings have been modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().bs().get((getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp)) {
    Cache::getInstance().getState().bs().unmodified((getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp);
    setContinue(true);

    return;
  }

  EPABsNofsamplespersyncEvent bs;
    
  bs.hdr.set(MEPHeader::BS_NOF_SAMPLES_PER_SYNC_HDR, 1 << m_blp);
  bs.nof_samples_per_sync_interval = Cache::getInstance().getBack().getClock() * (uint32)1.0e6; // convert from MHz to Hz
  
  LOG_INFO(formatString("setting BS.NOF_SAMPLES_PER_SYNC_INTERVAL to %u on (BP=%u, AP=%u)",
			bs.nof_samples_per_sync_interval, getBoardId(), m_blp));
  
  m_hdr = bs.hdr;
  getBoardPort().send(bs);
}

void BSWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult BSWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_INFO("Received BS.NOF_SAMPLES_PER_SYNC_INTERVAL ack");

  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("BSWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("BSWrite::handleack: invalid ack");
    Cache::getInstance().getState().bs().write_error((getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp);
    return GCFEvent::NOT_HANDLED;
  }

  // change state to indicate that it has been applied in the hardware
  Cache::getInstance().getState().bs().write_ack((getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + m_blp);

  return GCFEvent::HANDLED;
}
