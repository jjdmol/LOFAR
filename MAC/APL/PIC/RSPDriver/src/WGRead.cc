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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "StationSettings.h"
#include "WGRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RTC;

WGRead::WGRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL * 2)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

WGRead::~WGRead()
{
  /* TODO: delete event? */
}

void WGRead::sendrequest()
{
  if (getCurrentIndex() < StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) {
    EPAReadEvent wgsettingsread;

    if (0 == getCurrentIndex() % MEPHeader::N_POL) {
      wgsettingsread.hdr.set(MEPHeader::DIAG_WGX_HDR,
			     1 << (getCurrentIndex() / MEPHeader::N_POL),
			     MEPHeader::READ);
    }
    else {
      wgsettingsread.hdr.set(MEPHeader::DIAG_WGY_HDR,
			     1 << (getCurrentIndex() / MEPHeader::N_POL),
			     MEPHeader::READ);
    }

    m_hdr = wgsettingsread.hdr;
    getBoardPort().send(wgsettingsread);
  }
  else {
    int current_blp = getCurrentIndex() - StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL;

    EPAReadEvent wgwaveread;

    if (0 == current_blp % MEPHeader::N_POL) {
      wgwaveread.hdr.set(MEPHeader::DIAG_WGXWAVE_HDR,
			 1 << (current_blp / MEPHeader::N_POL),
			 MEPHeader::READ);
    }
    else {
      wgwaveread.hdr.set(MEPHeader::DIAG_WGYWAVE_HDR,
			 1 << (current_blp / MEPHeader::N_POL),
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
  if (getCurrentIndex() < StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) {
    if (EPA_DIAG_WG != event.signal) {
      LOG_WARN("WGRead::handleack: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
    EPADiagWgEvent wgsettings(event);

    if (!wgsettings.hdr.isValidAck(m_hdr)) {
      LOG_ERROR("WGRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

    uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    if (0 == GET_CONFIG("RSPDriver.LOOPBACK_MODE", i)) {
      if (w()(global_rcu).freq != wgsettings.freq
	  || w()(global_rcu).phase != wgsettings.phase
	  || w()(global_rcu).ampl != wgsettings.ampl
	  || w()(global_rcu).nof_samples != wgsettings.nof_samples
	  || w()(global_rcu).mode != wgsettings.mode)
      {
	LOG_WARN(formatString("LOOPBACK CHECK FAILED: WGRead mismatch (rcu=%d)", global_rcu));
      }
    }
    else {
      w()(global_rcu).freq        = wgsettings.freq;
      w()(global_rcu).phase       = wgsettings.phase;
      w()(global_rcu).ampl        = wgsettings.ampl;
      w()(global_rcu).nof_samples = wgsettings.nof_samples;
      w()(global_rcu).mode        = wgsettings.mode;
    }
  }
  else {
    int current_blp = getCurrentIndex() - StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL;
    
    if (EPA_DIAG_WGWAVE != event.signal) {
      LOG_WARN("WGRead::handleack: unexpected ack");
      return GCFEvent::NOT_HANDLED;
    }
  
    EPADiagWgwaveEvent wgwave(event);

    if (!wgwave.hdr.isValidAck(m_hdr)) {
      LOG_ERROR("WGRead::handleack: invalid ack");
      return GCFEvent::NOT_HANDLED;
    }

    uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + current_blp;

    WGSettings& w = Cache::getInstance().getBack().getWGSettings();

    Array<int32, 1> wave((int32*)&wgwave.samples,
			 shape(MEPHeader::N_WAVE_SAMPLES),
			 neverDeleteData);
    w.waveforms()(global_rcu, Range::all()) = wave;
  }
  
  return GCFEvent::HANDLED;
}
