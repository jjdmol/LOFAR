//#  WGRead.cc: implementation of the WGRead class
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

#include "WGRead.h"
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
using namespace blitz;

WGRead::WGRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL * 2)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

WGRead::~WGRead()
{
  /* TODO: delete event? */
}

void WGRead::sendrequest()
{
  if (getCurrentBLP() < GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
  {
    EPAReadEvent wgsettingsread;

    if (0 == getCurrentBLP() % MEPHeader::N_POL)
    {
      wgsettingsread.hdr.set(MEPHeader::WG_XSETTINGS_HDR,
			     getCurrentBLP() / 2,
			     MEPHeader::READ);
    }
    else
    {
      wgsettingsread.hdr.set(MEPHeader::WG_YSETTINGS_HDR,
			     getCurrentBLP() / 2,
			     MEPHeader::READ);
    }

    m_hdr = wgsettingsread.hdr;
    getBoardPort().send(wgsettingsread);
  }
  else
  {
    int current_blp = getCurrentBLP() - GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;

    EPAReadEvent wgwaveread;

    if (0 == current_blp % MEPHeader::N_POL)
    {
      wgwaveread.hdr.set(MEPHeader::WG_XWAVE_HDR,
			 current_blp / 2,
			 MEPHeader::READ);
    }
    else
    {
      wgwaveread.hdr.set(MEPHeader::WG_YWAVE_HDR,
			 current_blp / 2,
			 MEPHeader::READ);
    }

    m_hdr = wgwaveread.hdr;
    getBoardPort().send(wgwaveread);
  }
}

void WGRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult WGRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (getCurrentBLP() < GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
  {
    if (EPA_WG_SETTINGS != event.signal)
    {
      LOG_WARN("WGRead::handleack: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
    EPAWgSettingsEvent wgsettings(event);

    if (!wgsettings.hdr.isValidAck(m_hdr))
    {
      LOG_ERROR("WGRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

    uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + getCurrentBLP();

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i))
    {
      if (w()(global_rcu).freq != wgsettings.freq
	  || w()(global_rcu).phase != wgsettings.phase
	  || w()(global_rcu).ampl != wgsettings.ampl
	  || w()(global_rcu).nof_samples != wgsettings.nof_samples
	  || w()(global_rcu).mode != wgsettings.mode)
      {
	LOG_WARN(formatString("LOOPBACK CHECK FAILED: WGRead mismatch (rcu=%d)", global_rcu));
      }
    }
    else
    {
      w()(global_rcu).freq        = wgsettings.freq;
      w()(global_rcu).phase       = wgsettings.phase;
      w()(global_rcu).ampl        = wgsettings.ampl;
      w()(global_rcu).nof_samples = wgsettings.nof_samples;
      w()(global_rcu).mode        = wgsettings.mode;
    }
  }
  else
  {
    int current_blp = getCurrentBLP() - GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL;
    
    if (EPA_WG_WAVE != event.signal)
    {
      LOG_WARN("WGRead::handleack: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
    EPAWgWaveEvent wgwave(event);

    if (!wgwave.hdr.isValidAck(m_hdr))
    {
      LOG_ERROR("WGRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

    uint8 global_rcu = (getBoardId() * GET_CONFIG("RS.N_BLPS", i)) + current_blp;

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    Array<int16, 1> wave((int16*)&wgwave.samples,
			 shape(N_WAVE_SAMPLES),
			 neverDeleteData);
    w.waveforms()(global_rcu, Range::all()) = wave;
  }
  
  return GCFEvent::HANDLED;
}
