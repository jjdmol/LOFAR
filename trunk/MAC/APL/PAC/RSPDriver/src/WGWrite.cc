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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "WGWrite.h"
#include "EPA_Protocol.ph"
#include "RSP_Protocol.ph"
#include "Cache.h"

#include <PSAccess.h>

using namespace RSP;
using namespace LOFAR;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace blitz;
using namespace RTC;

WGWrite::WGWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL /** 2*/)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

WGWrite::~WGWrite()
{
  /* TODO: delete event? */
}

void WGWrite::sendrequest()
{
  if (getCurrentBLP() < GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
  {
    if (!Cache::getInstance().getBack().getWGSettings().getModified())
    {
      setCompleted(true);
      return;
    }
  
    uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

    EPAWgSettingsEvent wgsettings;

    if (0 == global_rcu % MEPHeader::N_POL)
    {
      wgsettings.hdr.set(MEPHeader::WG_XSETTINGS_HDR, getCurrentBLP() / 2);
    }
    else
    {
      wgsettings.hdr.set(MEPHeader::WG_YSETTINGS_HDR, getCurrentBLP() / 2);
    }

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    wgsettings.freq        = w()(global_rcu).freq;
    wgsettings.phase       = w()(global_rcu).phase;
    wgsettings.ampl        = w()(global_rcu).ampl;
    wgsettings.nof_samples = w()(global_rcu).nof_samples;
    wgsettings.mode        = w()(global_rcu).mode;
  
    m_hdr = wgsettings.hdr;
    getBoardPort().send(wgsettings);
  }
  else
  {
    if (!Cache::getInstance().getBack().getWGSettings().getModified())
    {
      setCompleted(true);
      return;
    }
 
    int current_blp = getCurrentBLP() - GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
    uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + current_blp;

    EPAWgWaveEvent wgwave;

    if (0 == global_rcu % MEPHeader::N_POL)
    {
      wgwave.hdr.set(MEPHeader::WG_XWAVE_HDR, current_blp / 2);
    }
    else
    {
      wgwave.hdr.set(MEPHeader::WG_YWAVE_HDR, current_blp / 2);
    }

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    // copy waveform to event to be sent
    Array<int16, 1> wave((int16*)&wgwave.samples,
			 shape(N_WAVE_SAMPLES),
			 neverDeleteData);

    wave = WGSettings::preset(w()(global_rcu).preset);
  
    m_hdr = wgwave.hdr;
    getBoardPort().send(wgwave);
  }
  
}

void WGWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult WGWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("WGWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("WGWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }
    
  return GCFEvent::HANDLED;
}
