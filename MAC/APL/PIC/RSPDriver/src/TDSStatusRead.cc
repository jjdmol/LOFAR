//#  TDSStatusRead.cc: implementation of the TDSStatusRead class
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

#include "TDSStatusRead.h"
#include "TDSi2cdefs.h"
#include "Cache.h"
#include "StationSettings.h"

#include <netinet/in.h>
#include <blitz/array.h>

#define STATUS_INDEX 0
#define READOK_INDEX 1

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;
using namespace RTC;

TDSStatusRead::TDSStatusRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TDSStatusRead::~TDSStatusRead()
{
  /* TODO: delete event? */
}

void TDSStatusRead::sendrequest()
{
  TDStatus& status = Cache::getInstance().getBack().getTDStatus();

  // always perform this action

  uint32 tds_control = 0;
  sscanf(GET_CONFIG_STRING("RSPDriver.TDS_CONTROL"), "%x", &tds_control);

  if (!(tds_control & (1 << getBoardId()))) {

    status.board()(getBoardId()).invalid = 1;
    Cache::getInstance().getState().tdstatusread().read_ack(getBoardId());
    setContinue(true);

    return;
  }

  // start with unknown status
  status.board()(getBoardId()).unknown = 1;

  // send read event
  EPATdsResultEvent tdsresult;
  tdsresult.hdr.set(MEPHeader::TDS_RESULT_HDR, MEPHeader::DST_RSP, MEPHeader::READ, sizeof(tds_readstatus_result));
  m_hdr = tdsresult.hdr; // remember header to match with ack
  getBoardPort().send(tdsresult);
}

void TDSStatusRead::sendrequest_status()
{
  // intentionally left empty
}

#if 0
void printbin(void* buf, int n)
{
  unsigned char* c = (unsigned char*)buf;
  fprintf(stderr, "TDSStatusRead: ");
  for (int i = 0; i< n; i++) {
    fprintf(stderr, "%02x ", c[i]);
  }
  fprintf(stderr, "\n");
}
#else
#define printbin(buf, n) do { } while (0)
#endif

GCFEvent::TResult TDSStatusRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_TDS_RESULT != event.signal) {
    LOG_WARN("TDSStatusRead::handleack:: unexpected ack");

    return GCFEvent::NOT_HANDLED;
  }
  
  EPATdsResultEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr)) {
    LOG_ERROR("TDSStatusRead::handleack: invalid ack");

    return GCFEvent::NOT_HANDLED;
  }

  printbin(ack.result, sizeof(tds_readstatus_result));

  TDStatus& status = Cache::getInstance().getBack().getTDStatus();

  if (ack.result[READOK_INDEX]) {
    // indicate that status is unknown
    status.board()(getBoardId()).unknown = 1;
    Cache::getInstance().getState().tdstatusread().read_error(getBoardId());
  } else {
    LOG_DEBUG(formatString("LOCK: 0x%02x", ack.result[STATUS_INDEX]));
    memcpy(&status.board()(getBoardId()), &ack.result + STATUS_INDEX, sizeof(TDBoardStatus));
    Cache::getInstance().getState().tdstatusread().read_ack(getBoardId());
  }

  return GCFEvent::HANDLED;
}
