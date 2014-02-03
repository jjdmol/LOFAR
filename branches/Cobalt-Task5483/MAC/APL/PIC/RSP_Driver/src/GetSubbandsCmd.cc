//#  GetSubbandsCmd.cc: implementation of the GetSubbandsCmd class
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
#include "GetSubbandsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

GetSubbandsCmd::GetSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("GetSubbands", port, oper)
{
  m_event = new RSPGetsubbandsEvent(event);
}

GetSubbandsCmd::~GetSubbandsCmd()
{
  delete m_event;
}

void GetSubbandsCmd::ack(CacheBuffer& cache)
{
	RSPGetsubbandsackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status    = RSP_SUCCESS;

	Range src_range;
	Range dst_range;
	int input_rcu;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());
	
	switch (m_event->type) {
	case SubbandSelection::BEAMLET:
		ack.subbands.beamlets().resize(m_event->rcumask.count(), nBanks, maxBeamletsPerPlane(cache.getBitsPerSample()));
		
    	input_rcu = 0;
    	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
    		if (m_event->rcumask[cache_rcu]) {
    			// NOTE: MEPHeader::N_BEAMLETS = 4x62 but userside MAX_BEAMLETS may be different
    			//       In other words: getSubbandSelection can contain more data than ack.weights
    			int nrSubbands = ack.subbands.beamlets().extent(thirdDim);
				int nrBlocks = MEPHeader::N_SERDES_LANES * nBanks;
				
				for (int block = 0; block < nrBlocks; block++) {
					int swbank = block / MEPHeader::N_SERDES_LANES;
					int swlane = block % MEPHeader::N_SERDES_LANES;
					int hwbank = block % nBanks;
					int hwlane = block / nBanks;
					int	swstart(swlane * maxDataslotsPerRSP(cache.getBitsPerSample()));
					int hwstart(hwlane * (MEPHeader::N_BEAMLETS/MEPHeader::N_SERDES_LANES));
					int nrSubbands2move(MIN(nrSubbands-swstart, maxDataslotsPerRSP(cache.getBitsPerSample())));
					if (nrSubbands2move > 0) {
						src_range = Range(hwstart, hwstart+nrSubbands2move-1);
						dst_range = Range(swstart, swstart+nrSubbands2move-1);
						//cache.getSubbandSelection().beamlets()(cache_rcu, hwbank, dst_range) = 0;

						ack.subbands.beamlets()(input_rcu, swbank, dst_range) =
						    cache.getSubbandSelection().beamlets()(cache_rcu, hwbank, src_range); //* (int)N_POL + (cache_rcu % N_POL)

						if (cache_rcu == 0) {
							LOG_DEBUG_STR("SS:block=" << block << " move(" << hwstart << ".." << hwstart+nrSubbands2move << ") to (" 
															  << swstart << ".." << swstart+nrSubbands2move << ")"
															  << " swbank:" << swbank << " swlane:" << swlane 
															  << " hwbank:" << hwbank << " hwlane:" << hwlane);
						}
					} // subbands left
				} // for each block
				
    			LOG_DEBUG_STR("GetSubbands:beamlet(cache[0]): " << cache.getSubbandSelection().beamlets()(0, Range::all(), Range::all()));
    			input_rcu++;
    		} // if rcu selected
    	} // for each rcu
		break;

	case SubbandSelection::XLET:
		ack.subbands.crosslets().resize(m_event->rcumask.count(), nBanks, MEPHeader::N_LOCAL_XLETS);
		
		input_rcu = 0;
    	for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
    		if (m_event->rcumask[cache_rcu]) {
    			if (m_event->type == SubbandSelection::XLET) {
    			    for (int bank = 0; bank < nBanks; bank++) {
        				ack.subbands.crosslets()(input_rcu, bank, Range::all()) = cache.getSubbandSelection().crosslets()(cache_rcu, bank, Range::all());
        			}
    			}
    			LOG_DEBUG_STR("GetSubbands:crosslets(cache[0]): " << cache.getSubbandSelection().crosslets()(0, Range::all(), Range::all()));
    			input_rcu++;
    		} // if rcu selected
    	} // for each rcu
		break;

	default:
		LOG_FATAL("invalid subbandselection type");
		exit(EXIT_FAILURE);
		break;
	}
	getPort()->send(ack);
}

void GetSubbandsCmd::apply(CacheBuffer& /*cache*/, bool /*setModFlag*/)
{
  /* intentionally left empty */
}

void GetSubbandsCmd::complete(CacheBuffer& cache)
{
  ack(cache);
}

const Timestamp& GetSubbandsCmd::getTimestamp() const
{
  return m_event->timestamp;
}

void GetSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
  m_event->timestamp = timestamp;
}

bool GetSubbandsCmd::validate() const
{
  return ((m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
	  && (m_event->type == SubbandSelection::BEAMLET || m_event->type == SubbandSelection::XLET));
}

bool GetSubbandsCmd::readFromCache() const
{
  return m_event->cache;
}
