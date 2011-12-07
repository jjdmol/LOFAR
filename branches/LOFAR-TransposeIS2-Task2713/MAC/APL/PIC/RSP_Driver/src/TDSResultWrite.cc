//#  TDSResultWrite.cc: implementation of the TDSResultWrite class
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

#include "TDSResultWrite.h"
#include "TDSi2cdefs.h"
#include "Cache.h"
#include "StationSettings.h"

#include <netinet/in.h>
#include <blitz/array.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;
using namespace RTC;

TDSResultWrite::TDSResultWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));

  // this action should be performed at initialisation
  doAtInit();
}

TDSResultWrite::~TDSResultWrite()
{
  /* TODO: delete event? */
}

void TDSResultWrite::sendrequest()
{
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().tdclear().get(getBoardId())) {
    Cache::getInstance().getState().tdclear().unmodified(getBoardId());
    setContinue(true);

    return;
  }

  uint32 tds_control = 0;
  sscanf(GET_CONFIG_STRING("RSPDriver.TDS_CONTROL"), "%x", &tds_control);

  if (!(tds_control & (1 << getBoardId()))) {
    Cache::getInstance().getState().tdclear().write_ack(getBoardId());
    setContinue(true);

    return;
  }

  EPATdsResultEvent tdsresult;
  tdsresult.hdr.set(MEPHeader::TDS_RESULT_HDR, MEPHeader::DST_RSP, MEPHeader::WRITE, MEPHeader::TDS_RESULT_SIZE);
  memset(tdsresult.result, 0xFF, MEPHeader::TDS_RESULT_SIZE);
  m_hdr = tdsresult.hdr; // remember header to match with ack

  // send write event
  LOG_INFO_STR("Clearing TDSH Protocol results register on board " << getBoardId());

  getBoardPort().send(tdsresult);
}

void TDSResultWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TDSResultWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal) {
    LOG_WARN("TDSResultWrite::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPATdsResultEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("TDSResultWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  // bail out if this is no a write ack
  if (!MEPHeader::WRITEACK == ack.hdr.m_fields.type) {
    LOG_ERROR("TDSResultWrite::handleack: invalid ack");
    Cache::getInstance().getState().tdclear().write_error(getBoardId());
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().tdclear().write_ack(getBoardId());

  return GCFEvent::HANDLED;
}
