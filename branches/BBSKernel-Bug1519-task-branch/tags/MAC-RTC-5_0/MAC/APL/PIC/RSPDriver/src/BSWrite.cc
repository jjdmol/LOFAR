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

#include "BSWrite.h"
#include "Cache.h"

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BSWrite::BSWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

BSWrite::~BSWrite()
{
  /* TODO: delete event? */
}

void BSWrite::sendrequest()
{
  // skip update if the neither of the RCU's settings have been modified
  if (!Cache::getInstance().getBack().getClocks().getModified())
  {
    setContinue(true);
    return;
  }

  EPABsNofsamplespersyncEvent bs;

  bs.hdr.set(MEPHeader::BS_NOF_SAMPLES_PER_SYNC_HDR, MEPHeader::DST_ALL_BLPS);
  bs.nof_samples_per_sync_interval = Cache::getInstance().getBack().getClocks()()(0);

  LOG_INFO(formatString("setting BS.NOF_SAMPLES_PER_SYNC_INTERVAL to %d on all BLPs", bs.nof_samples_per_sync_interval));

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
    return GCFEvent::NOT_HANDLED;
  }

  return GCFEvent::HANDLED;
}
