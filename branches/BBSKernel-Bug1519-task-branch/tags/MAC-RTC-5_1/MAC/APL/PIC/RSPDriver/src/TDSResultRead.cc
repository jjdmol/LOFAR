//#  TDSResultRead.cc: implementation of the TDSResultRead class
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

#include "TDSResultRead.h"
#include "TDSi2cdefs.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

TDSResultRead::TDSResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TDSResultRead::~TDSResultRead()
{
  /* TODO: delete event? */
}

void TDSResultRead::sendrequest()
{
  // skip update if the Clocks settings have not been modified
  if (RTC::RegisterState::APPLIED != Cache::getInstance().getBack().getClocks().getState().get(getBoardId()))
  {
    setContinue(true);
    return;
  }

  // delay 2 periods
  static int delay = 0;
  if (delay++ < 2) {
    setContinue(true);
    return;
  }
  delay = 0;

  EPATdsResultEvent tdsresult;
  tdsresult.hdr.set(MEPHeader::TDS_RESULT_HDR, MEPHeader::DST_RSP, MEPHeader::READ, sizeof(tds_160MHz_result)); // same sizeof for tds_200MHz_result
  memset(tdsresult.result, 0, MEPHeader::TDS_RESULT_SIZE);

  m_hdr = tdsresult.hdr; // remember header to match with ack
  getBoardPort().send(tdsresult);
}

void TDSResultRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TDSResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_TDS_RESULT != event.signal)
    {
      LOG_WARN("TDSResultRead::handleack:: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
  EPATdsResultEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
    {
      LOG_ERROR("TDSResultRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

  // TODO? copy bytes to match into tds_periodic_result?

  if (Cache::getInstance().getBack().getClocks()()(getBoardId()) == 160) {

    if (0 == memcmp(tds_160MHz_result, ack.result, sizeof(tds_160MHz_result))) {
      Cache::getInstance().getBack().getClocks().getState().confirmed(getBoardId());
    } else {
      LOG_ERROR("TDSResultRead::handleack: unexpected I2C result response");
    }
 
  } else if (Cache::getInstance().getBack().getClocks()()(getBoardId()) == 200) {

    if (0 == memcmp(tds_200MHz_result, ack.result, sizeof(tds_200MHz_result))) {
      Cache::getInstance().getBack().getClocks().getState().confirmed(getBoardId());
    } else {
      LOG_ERROR("TDSResultRead::handleack: unexpected I2C result response");
    }

  } else {

    if (0 == memcmp(tds_off_result, ack.result, sizeof(tds_off_result))) {
      Cache::getInstance().getBack().getClocks().getState().confirmed(getBoardId());
    } else {
      LOG_ERROR("TDSResultRead::handleack: unexpected I2C result response");
    }

  }

  return GCFEvent::HANDLED;
}
