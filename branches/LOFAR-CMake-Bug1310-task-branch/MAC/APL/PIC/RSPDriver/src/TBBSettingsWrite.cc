//#  TBBSettingsWrite.cc: implementation of the TBBSettingsWrite class
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "TBBSettingsWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

TBBSettingsWrite::TBBSettingsWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrRcusPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

TBBSettingsWrite::~TBBSettingsWrite()
{
  /* TODO: delete event? */
}

void TBBSettingsWrite::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  // skip update if not modified
  if (RTC::RegisterState::WRITE != Cache::getInstance().getState().tbbsettings().get(global_rcu)) {
    Cache::getInstance().getState().tbbsettings().unmodified(global_rcu);
    setContinue(true);
    return;
  }
   
  LOG_DEBUG(formatString(">>>> TBBSettingsWrite(%s) global_rcu=%d",
			 getBoardPort().getName().c_str(),
			 global_rcu));

  // send TBB settings
  EPATbbSettingsEvent tbbsettings;
  tbbsettings.hdr.set(MEPHeader::WRITE,
		      1 << (getCurrentIndex() / MEPHeader::N_POL),
		      MEPHeader::TBB,
		      MEPHeader::TBB_SETTINGSX + (getCurrentIndex() % MEPHeader::N_POL),
		      MEPHeader::TBB_SETTINGS_SIZE);
		      
  tbbsettings.stationid = GET_CONFIG("RS.STATION_ID", i);
  tbbsettings.rspid = getBoardId();
  tbbsettings.rcuid = global_rcu;
  tbbsettings.sample_freq = Cache::getInstance().getBack().getSystemStatus().board()(getBoardId()).rsp.bp_clock; // copy from status register

  tbbsettings.nof_bands = Cache::getInstance().getBack().getTBBSettings()()(global_rcu).count();
  if (0 == tbbsettings.nof_bands) {
    // store transient data
    tbbsettings.nof_samples = MEPHeader::TBB_NTRANSIENTSAMPLES;
  } else {
    // fit as many complete spectra of tbbsettings.nof_bands in each TBB frame
    tbbsettings.nof_samples = MIN((int)std::floor((double)(MEPHeader::TBB_MAXPAYLOADSIZE / MEPHeader::TBB_SPECTRALSAMPLESIZE) /
						   tbbsettings.nof_bands) * tbbsettings.nof_bands,
				  MEPHeader::TBB_MAXPAYLOADSIZE / MEPHeader::TBB_SPECTRALSAMPLESIZE);
  }

  m_hdr = tbbsettings.hdr;
  getBoardPort().send(tbbsettings);
}

void TBBSettingsWrite::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult TBBSettingsWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_WRITEACK != event.signal)
  {
    LOG_WARN("TBBSettingsWrite::handleack: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }

  EPAWriteackEvent ack(event);

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  if (!ack.hdr.isValidAck(m_hdr))
  {
    Cache::getInstance().getState().tbbsettings().write_error(global_rcu);

    LOG_ERROR("TBBSettingsWrite::handleack: invalid ack");
    return GCFEvent::NOT_HANDLED;
  }

  Cache::getInstance().getState().tbbsettings().write_ack(global_rcu);
  
  return GCFEvent::HANDLED;
}
