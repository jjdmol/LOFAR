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
  Cache::getInstance().getState().sys().read(getBoardId());

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
  if (EPA_RSR_STATUS != event.signal) {
      LOG_WARN("StatusRead::handleack: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }

  EPARsrStatusEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr)) {
      Cache::getInstance().getState().sys().read_error(getBoardId());
      LOG_ERROR("StatusRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

  SystemStatus& status = Cache::getInstance().getBack().getSystemStatus();

  // copy board status
  memcpy(&status.board()(getBoardId()), &ack.board, sizeof(BoardStatus));

  // sanity check on SYNC status, status for all AP's must be the same
  if (ack.board.ap0_sync.sample_offset != ack.board.ap1_sync.sample_offset
      || ack.board.ap0_sync.sample_offset != ack.board.ap2_sync.sample_offset
      || ack.board.ap0_sync.sample_offset != ack.board.ap3_sync.sample_offset) {
      LOG_WARN(formatString("RSP[%02d]: sample_offset mismatch", getBoardId()));
    }

  if (ack.board.ap0_sync.sync_count != ack.board.ap1_sync.sync_count
      || ack.board.ap0_sync.sync_count != ack.board.ap2_sync.sync_count
      || ack.board.ap0_sync.sync_count != ack.board.ap3_sync.sync_count) {
      LOG_WARN(formatString("RSP[%02d]: sync_count mismatch", getBoardId()));
    }

  if (ack.board.ap0_sync.slice_count != ack.board.ap1_sync.slice_count
      || ack.board.ap0_sync.slice_count != ack.board.ap2_sync.slice_count
      || ack.board.ap0_sync.slice_count != ack.board.ap3_sync.slice_count) {
      LOG_WARN(formatString("RSP[%02d]: slice_count mismatch", getBoardId()));
    }

  Cache::getInstance().getState().sys().read_ack(getBoardId());

  // if cache value different from hardware reported value, make equal
  switch (ack.board.rsp.bp_clock) {

  case 160:
  case 200:
    if (0 == getBoardId()) {
      if (0 == Cache::getInstance().getBack().getClock()) {
#if 0
	LOG_INFO_STR(formatString("Receiving initial clock setting from RSP board: %d MHz. Adjusting cache value.",
				  ack.board.rsp.bp_clock));
	Cache::getInstance().getFront().getClock() = ack.board.rsp.bp_clock;
	Cache::getInstance().getBack().getClock()  = ack.board.rsp.bp_clock;
#endif
      } else if (ack.board.rsp.bp_clock != Cache::getInstance().getBack().getClock()) {
	LOG_WARN_STR(formatString("Reported clock (%d MHz) is different from cache settings (%d MHz) on RSP board %d",
				  ack.board.rsp.bp_clock, Cache::getInstance().getBack().getClock(), getBoardId()));
      }
    }
    break;

  default:
    LOG_WARN_STR(formatString("Invalid clock setting received from RSP board (%d): %d MHz",
			      getBoardId(), ack.board.rsp.bp_clock));
    break;
  }

  return GCFEvent::HANDLED;
}

