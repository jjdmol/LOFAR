//#  WGWrite.cc: implementation of the WGWrite class
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

#include "WGWrite.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

WGWrite::WGWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
{
}

WGWrite::~WGWrite()
{
  /* TODO: delete event? */
}

void WGWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

  EPAWgSettingsEvent wgsettings;

  if (0 == global_rcu % MEPHeader::N_POL)
  {
    wgsettings.hdr.set(MEPHeader::WG_XSETTINGS_HDR, getCurrentBLP());
  }
  else
  {
    wgsettings.hdr.set(MEPHeader::WG_YSETTINGS_HDR, getCurrentBLP());
  }

  WGSettings& w = Cache::getInstance().getBack().getWGSettings();

  wgsettings.freq        = w()(global_rcu).freq;
  wgsettings.phase       = w()(global_rcu).phase;
  wgsettings.ampl        = w()(global_rcu).ampl;
  wgsettings.nof_samples = w()(global_rcu).nof_samples;
  wgsettings.mode        = w()(global_rcu).mode;
  
//  memcpy(&wgsettings.freq, w()(blitz::Range(global_blp,global_blp)).data(), sizeof(WGSettings::WGRegisterType));

  getBoardPort().send(wgsettings);
}

void WGWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult WGWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_DEBUG("handleack");

  EPAWriteackEvent ack(event);

  if (ack.hdr.m_fields.error)
  {
    LOG_ERROR_STR("WGWrite::handleack: error " << ack.hdr.m_fields.error);
  }
  
  return GCFEvent::HANDLED;
}
