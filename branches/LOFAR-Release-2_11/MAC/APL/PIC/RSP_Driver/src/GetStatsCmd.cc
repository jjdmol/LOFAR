//#  GetStatsCmd.cc: implementation of the GetStatsCmd class
//#  GetStatsCmd.cc: implementation of the GetStatsCmd class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <Common/LofarBitModeInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetStatsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetStatsCmd::GetStatsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetStats", port, oper)
{
  m_event = new RSPGetstatsEvent(event);

  m_n_devices = ((m_event->type <= Statistics::SUBBAND_POWER) ? NR_BLPS_PER_RSPBOARD : 1)
												* StationSettings::instance()->nrRspBoards() * N_POL;
}

GetStatsCmd::~GetStatsCmd()
{
  delete m_event;
}

void GetStatsCmd::ack(CacheBuffer& cache)
{
	RSPGetstatsackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status    = RSP_SUCCESS;

	if (m_event->type <= Statistics::SUBBAND_POWER) {
		ack.stats().resize(m_event->rcumask.count(), cache.getSubbandStats()().extent(secondDim));
	}
	else {
		ack.stats().resize(m_event->rcumask.count(), maxBeamlets(cache.getBitsPerSample()));
	}

    int activePlanes = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());
	
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
				if (MEPHeader::N_BEAMLETS == maxBeamlets(cache.getBitsPerSample())) {
					ack.stats()(result_device, Range::all()) = cache.getBeamletStats()()(cache_device, Range::all());
				}
				else {
					for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
    					for (int plane = 0; plane < activePlanes; plane++) {
    						int	swstart(lane*maxBeamletsPerRSP(cache.getBitsPerSample()) + plane*maxDataslotsPerRSP(cache.getBitsPerSample()));
    						int hwstart(lane*MEPHeader::N_BEAMLETS + plane*MEPHeader::N_BEAMLETS/4);
    						
    						ack.stats()(result_device, Range(swstart,swstart+maxDataslotsPerRSP(cache.getBitsPerSample())-1)) = 
    							cache.getBeamletStats()()(cache_device, Range(hwstart, hwstart+maxDataslotsPerRSP(cache.getBitsPerSample())-1));
    						
    						if (cache_device == 0) {
    							LOG_DEBUG_STR("Getstats:move(" << hwstart << ".." << hwstart+maxDataslotsPerRSP(cache.getBitsPerSample()-1) << ") to (" 
    														   << swstart << ".." << swstart+maxDataslotsPerRSP(cache.getBitsPerSample()-1) << ")");
    						}
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

void GetStatsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void GetStatsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetStatsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetStatsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetStatsCmd::validate() const
{
  return ((m_event->rcumask.count() <= m_n_devices)
	  && (m_event->type < Statistics::N_STAT_TYPES));
}

bool GetStatsCmd::readFromCache() const
{
  return m_event->cache;
}

void GetStatsCmd::ack_fail()
{
  RSPGetstatsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_FAILURE;
  ack.stats().resize(0, 0, 0);

  getPort()->send(ack);
}
