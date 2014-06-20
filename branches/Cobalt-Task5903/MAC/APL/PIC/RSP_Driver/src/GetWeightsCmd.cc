//#  GetWeightsCmd.cc: implementation of the GetWeightsCmd class
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
#include <Common/LofarBitModeInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "GetWeightsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetWeightsCmd::GetWeightsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetWeights", port, oper)
{
  m_event = new RSPGetweightsEvent(event);
}

GetWeightsCmd::~GetWeightsCmd()
{
  delete m_event;
}

void GetWeightsCmd::ack(CacheBuffer& cache)
{
	RSPGetweightsackEvent ack;
    int nPlanes = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());
	
	ack.timestamp = getTimestamp();
	ack.status    = RSP_SUCCESS;
	ack.weights().resize(BeamletWeights::SINGLE_TIMESTEP, m_event->rcumask.count(), nPlanes, maxBeamletsPerPlane(cache.getBitsPerSample()));	// 4 x 61

    Range	dst_range;
	Range	src_range;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());

	int input_rcu = 0;
	
	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
		if (m_event->rcumask[cache_rcu]) {
			// NOTE: MEPHeader::N_BEAMLETS = 4x62 but userside MAX_BEAMLETS may be different
			//       In other words: getBeamletWeights can contain more data than ack.weights
		    int nrBlocks = MEPHeader::N_SERDES_LANES * nBanks;
			int dataslotsPerRSP = maxDataslotsPerRSP(cache.getBitsPerSample());
			for (int block = 0; block < nrBlocks; block++) {
				int swbank = block / MEPHeader::N_SERDES_LANES;
				int swlane = block % MEPHeader::N_SERDES_LANES;
				int hwbank = block % nBanks;
				int hwlane = block / nBanks;
				int	swstart(swlane * dataslotsPerRSP);
				int hwstart(hwlane * (MEPHeader::N_BEAMLETS/MEPHeader::N_SERDES_LANES));
				src_range = Range(hwstart, hwstart+dataslotsPerRSP-1);
				dst_range = Range(swstart, swstart+dataslotsPerRSP-1);
				for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
					ack.weights()(0, input_rcu, swbank, dst_range) = 
					    cache.getBeamletWeights()()(0, cache_rcu, hwbank, src_range); 
					if (lane == 0) {
						LOG_DEBUG_STR("BF:block=" << block << " move(" << src_range << ") to (" << dst_range << ")"
									<< " swbank=" << swbank << " swlane=" << swlane
									<< " hwbank=" << hwbank << " hwlane=" << hwlane);
					}
				} // lanes
			} // blocks
			
		    
			input_rcu++;
			if (cache_rcu ==0) {
				LOG_DEBUG_STR("GetWeights(ack[0]): " << ack.weights()(0,0,Range::all(),Range::all()));
			}
		}
	}
	getPort()->send(ack);
}

void GetWeightsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  // no-op
}

void GetWeightsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetWeightsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetWeightsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetWeightsCmd::validate() const
{
  return (m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus());
}

bool GetWeightsCmd::readFromCache() const
{
  return m_event->cache;
}
