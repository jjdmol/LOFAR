//#  SDOModeWrite.cc: implementation of the SDOModeWrite class
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
//#  $Id: SDOModeWrite.cc 18124 2011-05-29 19:54:09Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SDOModeWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SDOModeWrite::SDOModeWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
  memset(&itsHdr, 0, sizeof(MEPHeader));
  doAtInit();
}

SDOModeWrite::~SDOModeWrite()
{
  /* TODO: delete event? */
}

void SDOModeWrite::sendrequest()
{
  if (StationSettings::instance()->hasAartfaac() == false) {
    LOG_DEBUG_STR("SDOModeWrite:: No Aartfaac on this station");
    Cache::getInstance().getState().sdoState().read_ack(getBoardId());
    setContinue(true); // continue with next action
    setFinished();
    return;
  }
  
  // skip update if the neither of the RCU's settings have been modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().sdoState().get(getBoardId())) {
    Cache::getInstance().getState().sdoState().unmodified(getBoardId());
    setContinue(true);
    return;
  }
  
  LOG_DEBUG(formatString(">>>> SDOModeWrite(%s) boardId=%d",
             getBoardPort().getName().c_str(),
             getBoardId()));
    
  // send sdo mode select message
  EPARsrSdomodeEvent sdomode;
  sdomode.hdr.set(MEPHeader::RSR_SDOMODE_HDR,
                  MEPHeader::DST_ALL);
    
  sdomode.sdomode.bm_select = Cache::getInstance().getBack().getSDOModeInfo()()(getBoardId()).bm_select;
  
  itsHdr = sdomode.hdr;
  LOG_INFO_STR(sdomode);
  getBoardPort().send(sdomode);
}

void SDOModeWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult SDOModeWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("SDOModeWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent sdomode(event);

  if (!sdomode.hdr.isValidAck(itsHdr))
  {
    Cache::getInstance().getState().sdoState().write_error(getBoardId());
    LOG_ERROR("SDOModeWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().sdoState().write_ack(getBoardId());
  
  return GCFEvent::HANDLED;
}
