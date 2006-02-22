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
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "StationSettings.h"
#include "WGWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RTC;

#define N_REGISTERS 2 // the number of registers to write (DiagWg and DiagWgwave)

WGWrite::WGWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL * N_REGISTERS)
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

WGWrite::~WGWrite()
{
  /* TODO: delete event? */
}

void WGWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard() * MEPHeader::N_POL) + (getCurrentIndex() / N_REGISTERS);

  if (RTC::RegisterState::NOT_MODIFIED == Cache::getInstance().getBack().getWGSettings().getState().get(global_rcu)) {
    setContinue(true);
    return;
  }

  switch (getCurrentIndex() % N_REGISTERS) {

  case 0:
    {
      EPADiagWgEvent wgsettings;

      if (0 == global_rcu % MEPHeader::N_POL) {
	wgsettings.hdr.set(MEPHeader::DIAG_WGX_HDR, 1 << (global_rcu / MEPHeader::N_POL));
      } else {
	wgsettings.hdr.set(MEPHeader::DIAG_WGY_HDR, 1 << (global_rcu / MEPHeader::N_POL));
      }

      WGSettings& w = Cache::getInstance().getBack().getWGSettings();

      wgsettings.mode        = w()(global_rcu).mode;
      wgsettings.phase       = w()(global_rcu).phase;
      wgsettings.nof_samples = w()(global_rcu).nof_samples;
      wgsettings.freq        = w()(global_rcu).freq;
      wgsettings.ampl        = w()(global_rcu).ampl;

      // wgsettings.preset field is not set because it is not sent to the hardware
      // (see struct definition in RSP_Protocol/include/APL/RSP_Protocol/WGSettings.h)
  
      m_hdr = wgsettings.hdr;
      getBoardPort().send(wgsettings);
    }
    break;

  case 1:
    {
      // Should we write the wave form also?
      if (GET_CONFIG("RSPDriver.WRITE_WG_WAVE", i) == 0) {
	setContinue(true);
	return;
      }

      EPADiagWgwaveEvent wgwave;

      if (0 == global_rcu % MEPHeader::N_POL)
	{
	  wgwave.hdr.set(MEPHeader::DIAG_WGXWAVE_HDR, 1 << (global_rcu / MEPHeader::N_POL));
	}
      else
	{
	  wgwave.hdr.set(MEPHeader::DIAG_WGYWAVE_HDR, 1 << (global_rcu / MEPHeader::N_POL));
	}

      WGSettings& w = Cache::getInstance().getBack().getWGSettings();

      // copy waveform to event to be sent
      Array<int32, 1> wave((int32*)&wgwave.samples,
			   shape(N_WAVE_SAMPLES),
			   neverDeleteData);

      wave = WGSettings::preset(w()(global_rcu).preset);
  
      m_hdr = wgwave.hdr;
      getBoardPort().send(wgwave);
    }
    break;
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
  
  // change state to indicate that it has been applied in the hardware
  Cache::getInstance().getBack().getWGSettings().getState().applied(getCurrentIndex() / N_REGISTERS);

  return GCFEvent::HANDLED;
}
