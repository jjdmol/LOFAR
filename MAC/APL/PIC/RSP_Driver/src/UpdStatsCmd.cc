//#  UpdStatsCmd.cc: implementation of the UpdStatsCmd class
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
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "UpdStatsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

UpdStatsCmd::UpdStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SubStats", port, oper)
{
  m_event = new RSPSubstatsEvent(event);
  m_n_devices = ((m_event->type <= Statistics::SUBBAND_POWER) ? NR_BLPS_PER_RSPBOARD : 1)
										* StationSettings::instance()->nrRspBoards() * N_POL;
  setPeriod(m_event->period);
}

UpdStatsCmd::~UpdStatsCmd()
{
  delete m_event;
}

void UpdStatsCmd::ack(CacheBuffer& /*cache*/)
{
  // intentionally left empty
}

void UpdStatsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void UpdStatsCmd::complete(CacheBuffer& cache)
{
//	LOG_INFO("UpdStatsCmd::complete");

	RSPUpdstatsEvent ack;

	ack.timestamp = getTimestamp();
	ack.status    = RSP_SUCCESS;
	ack.handle    = (memptr_t)this; // opaque pointer used to refer to the subscription

	if (m_event->type <= Statistics::SUBBAND_POWER) {
		ack.stats().resize(m_event->rcumask.count(), cache.getSubbandStats()().extent(secondDim));
	}
	else {
		ack.stats().resize(m_event->rcumask.count(), MAX_BEAMLETS);
	}

	unsigned int result_device = 0;
	for (unsigned int cache_device = 0; cache_device < m_n_devices; cache_device++) {
		if (m_event->rcumask[cache_device]) {
			switch (m_event->type) {
			case Statistics::SUBBAND_POWER:
				ack.stats()(result_device, Range::all()) = cache.getSubbandStats()()(cache_device, Range::all());
			break;

			case Statistics::BEAMLET_POWER:
				// NOTE: MEPHeader::N_BEAMLETS = 4x62 but userside MAX_BEAMLETS may be different
				//       In other words: getBeamletWeights can contain more data than ack.weights
				if (MEPHeader::N_BEAMLETS == MAX_BEAMLETS) {
					ack.stats()(result_device, Range::all()) = cache.getBeamletStats()()(cache_device, Range::all());
				}
				else {
					for (int rsp = 0; rsp < 4; rsp++) {
						int	swstart(rsp*MAX_BEAMLETS_PER_RSP);
						int hwstart(rsp*MEPHeader::N_BEAMLETS/4);
						ack.stats()(result_device, Range(swstart,swstart+MAX_BEAMLETS_PER_RSP-1)) = 
							cache.getBeamletStats()()(cache_device, Range(hwstart, hwstart+MAX_BEAMLETS_PER_RSP-1));
						if (cache_device == 0) {
							LOG_DEBUG_STR("Getstats:move(" << hwstart << ".." << hwstart+MAX_BEAMLETS_PER_RSP << ") to (" 
														   << swstart << ".." << swstart+MAX_BEAMLETS_PER_RSP << ")");
						}
					}
				}
				LOG_DEBUG_STR("GetStats(cache[0]): " << cache.getBeamletStats()()(0,Range::all()));
				
			break;

			default:
				LOG_ERROR("invalid statistics type");
			break;
			}

			result_device++;
		}
	}

	getPort()->send(ack);
}


const Timestamp& UpdStatsCmd::getTimestamp() const
{
//	LOG_INFO("UpdStatsCmd::getTimeStamp");
  return m_event->timestamp;
}

void UpdStatsCmd::setTimestamp(const Timestamp& timestamp)
{
//	LOG_INFO_STR("UpdStatsCmd::setTimeStamp:" << timestamp);
  m_event->timestamp = timestamp;
}

bool UpdStatsCmd::validate() const
{
//	LOG_INFO("UpdStatsCmd::validate");
  return ((m_event->rcumask.count() <= m_n_devices)
	  && (m_event->type < Statistics::N_STAT_TYPES));
}
