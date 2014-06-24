//#  SDOModeRead.cc: implementation of the SDOModeRead class
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
//#  $Id: SDOModeRead.cc 18124 2011-05-29 19:54:09Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SDOModeRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SDOModeRead::SDOModeRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&itsHdr, 0, sizeof(MEPHeader));
}

SDOModeRead::~SDOModeRead()
{
}

void SDOModeRead::sendrequest()
{
  if (StationSettings::instance()->hasAartfaac() == false) {
    LOG_DEBUG_STR(formatString("SDOModeRead:: No Aartfaac on this station", getBoardId()));
    Cache::getInstance().getState().sdoState().unmodified(getBoardId());
    setContinue(true); // continue with next action
    setFinished();
    return;
  }

  EPAReadEvent sdomoderead;
  sdomoderead.hdr.set(MEPHeader::RSR_SDOMODE_HDR,
                 MEPHeader::DST_RSP,
                 MEPHeader::READ );

  itsHdr = sdomoderead.hdr;
  getBoardPort().send(sdomoderead);
  LOG_DEBUG_STR("SDOModeRead::sendrequest() done");
}

void SDOModeRead::sendrequest_status()
{
  /* intentionally left empty */
}

GCFEvent::TResult SDOModeRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RSR_SDOMODE != event.signal)
  {
    LOG_WARN("SDOModeRead::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  // unpack bm message
  EPARsrSdomodeEvent sdo(event);

  LOG_DEBUG_STR(formatString("SDO supported = %d", sdo.sdomode.bm_max));
  LOG_DEBUG_STR(formatString("SDO selected  = %d", sdo.sdomode.bm_select));

  if (!sdo.hdr.isValidAck(itsHdr))
  {
    Cache::getInstance().getState().sdoState().read_error(getBoardId());
    LOG_ERROR("SDOModeRead::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  LOG_DEBUG("handleack");

  LOG_DEBUG(formatString(">>>> SDOModeRead(%s)", getBoardPort().getName().c_str()));
  
  Cache::getInstance().getBack().getSDOModeInfo()()(getBoardId()).bm_select = sdo.sdomode.bm_select;
  Cache::getInstance().getBack().getSDOModeInfo()()(getBoardId()).bm_max    = sdo.sdomode.bm_max;
  
  Cache::getInstance().getState().sdoState().read_ack(getBoardId());
    
  LOG_DEBUG_STR("SDOModeRead::handleack() done");
  return GCFEvent::HANDLED;
}
