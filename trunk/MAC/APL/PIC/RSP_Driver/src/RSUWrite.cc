//#  RSUWrite.cc: implementation of the RSUWrite class
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

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "RSUWrite.h"
#include "Cache.h"

// nof seconds to wait with writing BS register after RSU clear
// seems that original value 5 was too small, EPA team uses 7
// we want to be safe, so set it to 10 seconds
#define WRITE_BS_DELAY ((long)10)

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RTC;

static const EPA_Protocol::RSUReset  g_RSU_RESET_SYNC  = { 1, 0, 0, 0 }; // Soft SYNC
static const EPA_Protocol::RSUReset  g_RSU_RESET_CLEAR = { 0, 1, 0, 0 }; // CLEAR
static const EPA_Protocol::RSUReset  g_RSU_RESET_RESET = { 0, 0, 1, 0 }; // RESET
static const EPA_Protocol::RSUReset  g_RSU_RESET_NONE  = { 0, 0, 0, 0 }; // No action

RSUWrite::RSUWrite(GCFPortInterface& board_port, int board_id, const Scheduler& scheduler)
  : SyncAction(board_port, board_id, 1), m_scheduler(scheduler)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
  m_mark = Timestamp(0,0);

  // this action should be performed at initialisation
  doAtInit();
}

RSUWrite::~RSUWrite()
{
}

void RSUWrite::sendrequest()
{
  // cache modified, or initialising and clock update has completed
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().rsuclear().get(getBoardId())) {
    Cache::getInstance().getState().rsuclear().unmodified(getBoardId());
    setContinue(true);

    return;
  }

  EPARsuResetEvent reset;

  reset.hdr.set(MEPHeader::RSU_RESET_HDR);

  // read values from cache
  RSUSettings& s = Cache::getInstance().getBack().getRSUSettings();
  if (s()(getBoardId()).getSync()) {
    reset.reset = g_RSU_RESET_SYNC;

  } else if (s()(getBoardId()).getClear()) {
    reset.reset = g_RSU_RESET_CLEAR;

  } else if (s()(getBoardId()).getReset()) {
    reset.reset = g_RSU_RESET_RESET;

  } else {
    setContinue(true);
    return;
  }

  m_hdr = reset.hdr;
  getBoardPort().send(reset);
}

void RSUWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult RSUWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal) {
    LOG_WARN("RSUWrite::handleack: unexpected ack");

    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("RSUWrite::handleack: invalid ack");
    Cache::getInstance().getState().rsuclear().write_error(getBoardId());

    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().rsuclear().write_ack(getBoardId());

  Cache::getInstance().getBack().getRSUSettings()()(getBoardId()).setRaw(0); // clear the flags

  return GCFEvent::HANDLED;
}


