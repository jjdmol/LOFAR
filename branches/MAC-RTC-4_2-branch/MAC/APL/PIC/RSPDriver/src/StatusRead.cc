//#  StatusRead.cc: implementation of the StatusRead class
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
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <netinet/in.h>

#include "StatusRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RTC;

StatusRead::StatusRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

StatusRead::~StatusRead()
{
  /* TODO: delete event? */
}

void StatusRead::sendrequest()
{
  // send read status request to check status of the write
  EPAReadEvent rspstatus;
  rspstatus.hdr.set(MEPHeader::RSR_STATUS_HDR);

  m_hdr = rspstatus.hdr;
  getBoardPort().send(rspstatus);
}

void StatusRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult StatusRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RSR_STATUS != event.signal)
  {
    LOG_WARN("StatusRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPARsrStatusEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("StatusRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  SystemStatus& status = Cache::getInstance().getBack().getSystemStatus();

  // copy board status
  memcpy(&status.board()(getBoardId()), &ack.board, sizeof(BoardStatus));

  // sanity check on SYNC status, status for all AP's must be the same
  if (ack.board.ap0_sync.sample_offset != ack.board.ap1_sync.sample_offset
      || ack.board.ap0_sync.sample_offset != ack.board.ap2_sync.sample_offset
      || ack.board.ap0_sync.sample_offset != ack.board.ap3_sync.sample_offset)
    {
      LOG_WARN(formatString("RSP[%02d]: sample_offset mismatch", getBoardId()));
    }

  if (ack.board.ap0_sync.sync_count != ack.board.ap1_sync.sync_count
      || ack.board.ap0_sync.sync_count != ack.board.ap2_sync.sync_count
      || ack.board.ap0_sync.sync_count != ack.board.ap3_sync.sync_count)
    {
      LOG_WARN(formatString("RSP[%02d]: sync_count mismatch", getBoardId()));
    }

  if (ack.board.ap0_sync.slice_count != ack.board.ap1_sync.slice_count
      || ack.board.ap0_sync.slice_count != ack.board.ap2_sync.slice_count
      || ack.board.ap0_sync.slice_count != ack.board.ap3_sync.slice_count)
    {
      LOG_WARN(formatString("RSP[%02d]: slice_count mismatch", getBoardId()));
    }

  uint8 global_rcu_base = getBoardId() * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;

  // copy RCU status
  status.rcu()(global_rcu_base    ).status       = ack.board.ap0_rcu.statusx;
  status.rcu()(global_rcu_base    ).nof_overflow = ack.board.ap0_rcu.nof_overflowx;
  status.rcu()(global_rcu_base + 1).status       = ack.board.ap0_rcu.statusy;
  status.rcu()(global_rcu_base + 1).nof_overflow = ack.board.ap0_rcu.nof_overflowy;
  status.rcu()(global_rcu_base + 2).status       = ack.board.ap1_rcu.statusx;
  status.rcu()(global_rcu_base + 2).nof_overflow = ack.board.ap1_rcu.nof_overflowx;
  status.rcu()(global_rcu_base + 3).status       = ack.board.ap1_rcu.statusy;
  status.rcu()(global_rcu_base + 3).nof_overflow = ack.board.ap1_rcu.nof_overflowy;
  status.rcu()(global_rcu_base + 4).status       = ack.board.ap2_rcu.statusx;
  status.rcu()(global_rcu_base + 4).nof_overflow = ack.board.ap2_rcu.nof_overflowx;
  status.rcu()(global_rcu_base + 5).status       = ack.board.ap2_rcu.statusy;
  status.rcu()(global_rcu_base + 5).nof_overflow = ack.board.ap2_rcu.nof_overflowy;
  status.rcu()(global_rcu_base + 6).status       = ack.board.ap3_rcu.statusx;
  status.rcu()(global_rcu_base + 6).nof_overflow = ack.board.ap3_rcu.nof_overflowx;
  status.rcu()(global_rcu_base + 7).status       = ack.board.ap3_rcu.statusy;
  status.rcu()(global_rcu_base + 7).nof_overflow = ack.board.ap3_rcu.nof_overflowy;

  return GCFEvent::HANDLED;
}

