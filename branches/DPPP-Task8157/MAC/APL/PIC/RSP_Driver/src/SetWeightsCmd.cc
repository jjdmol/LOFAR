//#  SetWeightsCmd.cc: implementation of the SetWeightsCmd class
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
#include "SetWeightsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

SetWeightsCmd::SetWeightsCmd(RSPSetweightsEvent& sw_event, GCFPortInterface& port,
			     Operation oper, int timestep) :
	Command("SetWeights", port, oper)
{
  RSPSetweightsEvent* event = new RSPSetweightsEvent();
  m_event = event;
  
  event->timestamp = sw_event.timestamp + (long)timestep;
  event->rcumask   = sw_event.rcumask;
}

SetWeightsCmd::~SetWeightsCmd()
{
  delete m_event;
}

void SetWeightsCmd::setWeights(Array<complex<int16>, BeamletWeights::NDIM> weights)
{
  RSPSetweightsEvent* event = static_cast<RSPSetweightsEvent*>(m_event);
  
  event->weights().resize(BeamletWeights::SINGLE_TIMESTEP, event->rcumask.count(), weights.extent(thirdDim) ,weights.extent(fourthDim));
  event->weights() = weights;
}

void SetWeightsCmd::ack(CacheBuffer& /*cache*/)
{
  RSPSetweightsackEvent ack;

  ack.timestamp = getTimestamp();
  ack.status = RSP_SUCCESS;
  
  getPort()->send(ack);
}

void SetWeightsCmd::apply(CacheBuffer& cache, bool setModFlag)
{
	int input_rcu = 0;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());

	Range	src_range;
	Range	dst_range;
	
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
				dst_range = Range(hwstart, hwstart+dataslotsPerRSP-1);
				src_range = Range(swstart, swstart+dataslotsPerRSP-1);
				for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
					cache.getBeamletWeights()()(0, cache_rcu, hwbank, dst_range) = 
								m_event->weights()(0, input_rcu, swbank, src_range);
					if (lane == 0) {
						LOG_DEBUG_STR("BF:block=" << block << " move(" << src_range << ") to (" << dst_range << ")"
									<< " swbank=" << swbank << " swlane=" << swlane
									<< " hwbank=" << hwbank << " hwlane=" << hwlane);
					}
				} // lanes
			} // blocks

			if (setModFlag) {
				cache.getCache().getState().bf().write(cache_rcu * MEPHeader::N_PHASE);
				cache.getCache().getState().bf().write(cache_rcu * MEPHeader::N_PHASE + 1);
				if (cache_rcu == 0) {
					LOG_DEBUG_STR("SetWeights(cache[0]): " << cache.getBeamletWeights()()(0,0,Range::all(),Range::all()));
				}
			}

			input_rcu++;
		}
	}
}

void SetWeightsCmd::complete(CacheBuffer& /*cache*/)
{
//	LOG_INFO_STR("SetWeightsCmd completed at time=" << getTimestamp());
}

const Timestamp& SetWeightsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void SetWeightsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool SetWeightsCmd::validate() const
{
  // validation is done in the caller (RSPDriverTask)
  return true;
}

