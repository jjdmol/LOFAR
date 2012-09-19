//#  SetSubbandsCmd.cc: implementation of the SetSubbandsCmd class
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
#include "SetSubbandsCmd.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RSP_Protocol;
using namespace RTC;

// BITMODE 16
// bank 0:
// lane 0    lane 1    lane 2    lane 3
//	0,1      122,123   244,245   366,367
//  2,3      124,125   246,247   368,369
//   ..        ..        ..        ..
// 120,121   242,243   364,365,  486,487
//
// BITMODE 8
// bank 0:
// lane 0    lane 1    lane 2    lane 3
//	0,1      236,237   472,473   708,709
//  2,3      238,239   474,475   710,711
//   ..        ..        ..        ..
// 116,117   352,353   588,589,  824,825
//
// bank 1:
// lane 0    lane 1    lane 2    lane 3
// 118,119   354,355   590,591   826,827
// 120,121   356,357   592,593   828,829
//   ..        ..        ..        ..
// 234,235   470,471   706,707   942,943

SetSubbandsCmd::SetSubbandsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper) :
	Command("SetSubbands", port, oper)
{
	m_event = new RSPSetsubbandsEvent(event);
}

SetSubbandsCmd::~SetSubbandsCmd()
{
	delete m_event;
}

void SetSubbandsCmd::ack(CacheBuffer& /*cache*/)
{
	RSPSetsubbandsackEvent ack;

	ack.timestamp = getTimestamp();
	ack.status = RSP_SUCCESS;

	getPort()->send(ack);
}

void SetSubbandsCmd::apply(CacheBuffer& cache, bool /*setModFlag*/)
{
	#undef MIN
	#define	MIN(A,B) ((A)<(B)?(A):(B))

	Range	dst_range;
	Range	src_range;
	int nBanks = (MAX_BITS_PER_SAMPLE / cache.getBitsPerSample());

	switch (m_event->subbands.getType()) {

	case SubbandSelection::BEAMLET: {
		//dst_range = Range(MEPHeader::N_LOCAL_XLETS, MEPHeader::N_LOCAL_XLETS + MEPHeader::N_BEAMLETS - 1);
		dst_range = Range(0, m_event->subbands.beamlets().extent(thirdDim) - 1);
		for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
			if (m_event->rcumask[cache_rcu]) {
				// NOTE: MEPHeader::N_BEAMLETS = 4x62 but userside MAX_BEAMLETS may be different
				//       In other words: getSubbandSelection can contain more data than m_event->subbands
				if (MEPHeader::N_BEAMLETS == maxBeamlets(cache.getBitsPerSample())) {
					cache.getSubbandSelection().beamlets()(cache_rcu, 0, dst_range) = 0;
					cache.getSubbandSelection().beamlets()(cache_rcu, 0, dst_range) = 
											m_event->subbands.beamlets()(0, 0, Range::all()) * (int)N_POL + (cache_rcu % N_POL);
				}
				else {
					int nr_subbands = m_event->subbands.beamlets().extent(thirdDim);
					int nrBlocks = MEPHeader::N_SERDES_LANES * nBanks;
					for (int block = 0; block < nrBlocks; block++) {
						int swbank = block / MEPHeader::N_SERDES_LANES;
						int swlane = block % MEPHeader::N_SERDES_LANES;
						int hwbank = block % nBanks;
						int hwlane = block / nBanks;
    					int	swstart(swlane * maxDataslotsPerRSP(cache.getBitsPerSample()));
    					int hwstart(hwlane * (MEPHeader::N_BEAMLETS/MEPHeader::N_SERDES_LANES));
    					int nrSubbands2move(MIN(nr_subbands-swstart, maxDataslotsPerRSP(cache.getBitsPerSample())));
    					if (nrSubbands2move > 0) {
    						dst_range = Range(hwstart, hwstart+nrSubbands2move-1);
    						src_range = Range(swstart, swstart+nrSubbands2move-1);
    						cache.getSubbandSelection().beamlets()(cache_rcu, hwbank, dst_range) = 0;
    						cache.getSubbandSelection().beamlets()(cache_rcu, hwbank, dst_range) = 
										m_event->subbands.beamlets()(0, swbank, src_range) * (int)N_POL + (cache_rcu % N_POL);
    						if (cache_rcu == 0) {
    							LOG_DEBUG_STR("Setsubbands:move(" << swstart << ".." << swstart+nrSubbands2move << ") to (" 
    															  << hwstart << ".." << hwstart+nrSubbands2move << ")"
																  << " swbank:" << swbank << " swlane:" << swlane 
																  << " hwbank:" << hwbank << " hwlane:" << hwlane);
    						}
    					} // subbands left
    				} // for each block
				} // difference in max'en

				if (cache_rcu == 0) {
					LOG_DEBUG_STR("m_event->subbands.beamlets() = " << m_event->subbands.beamlets());
					LOG_DEBUG_STR("cache->subbands.beamlets(0) = " << cache.getSubbandSelection().beamlets()(0, Range::all(), Range::all()));
				}
			} // if rcu selected
		} // for each rcu
	} // case
	break;

	case SubbandSelection::XLET: {
		dst_range = Range(0, MEPHeader::N_LOCAL_XLETS - 1);
		for (int cache_rcu = 0; cache_rcu < StationSettings::instance()->nrRcus(); cache_rcu++) {
			if (m_event->rcumask[cache_rcu]) {
			    for (int bank = 0; bank < nBanks; bank++) {
				    cache.getSubbandSelection().crosslets()(cache_rcu, bank, dst_range) = 0;
				    cache.getSubbandSelection().crosslets()(cache_rcu, bank, dst_range) = m_event->subbands.crosslets()(0,0,0) * N_POL + (cache_rcu % N_POL);
                }
				LOG_DEBUG_STR("m_event->subbands.crosslets() = " << m_event->subbands.crosslets());
			}
		}
	}
	break;

	default:
		LOG_FATAL("invalid subbandselection type");
		exit(EXIT_FAILURE);
		break;
	}
}

void SetSubbandsCmd::complete(CacheBuffer& /*cache*/)
{
//	LOG_INFO_STR("SetSubbandsCmd completed at time=" << getTimestamp());
}

const Timestamp& SetSubbandsCmd::getTimestamp() const
{
	return m_event->timestamp;
}

void SetSubbandsCmd::setTimestamp(const Timestamp& timestamp)
{
	m_event->timestamp = timestamp;
}

bool SetSubbandsCmd::validate() const
{
	bool valid = false;

	switch (m_event->subbands.getType()) {

	case SubbandSelection::BEAMLET:
		if (   (m_event->subbands.beamlets().extent(thirdDim) <= MEPHeader::N_BEAMLETS)
			&& (3 == m_event->subbands.beamlets().dimensions())
		    && (1 == m_event->subbands.beamlets().extent(firstDim))) {
		    valid = true;
		}
		break;

	case SubbandSelection::XLET:
		if (   (1 == m_event->subbands.crosslets().extent(thirdDim))
	        && (3 == m_event->subbands.crosslets().dimensions())
		    && (1 == m_event->subbands.crosslets().extent(firstDim))) {
		    valid = true;
		}
		break;

	default:
		LOG_WARN("invalid SubbandSelection type or dimensions");
		break;
	}

	// return true when everything is right
	if (   (m_event->rcumask.count() <= (unsigned int)StationSettings::instance()->nrRcus())
		&& valid) {
		return (true);
	}

	// show our validation values.
    LOG_DEBUG(formatString("cmd rcumask.count    = %d",m_event->rcumask.count()));
    LOG_DEBUG(formatString("nr Rcus              = %d",StationSettings::instance()->nrRcus()));
    LOG_DEBUG(formatString("first dim crosslets  = %d",m_event->subbands.crosslets().extent(firstDim)));
    LOG_DEBUG(formatString("second dim crosslets = %d",m_event->subbands.crosslets().extent(secondDim)));
    LOG_DEBUG(formatString("thirth dim crosslets = %d",m_event->subbands.crosslets().extent(thirdDim)));
    LOG_DEBUG(formatString("first dim beamlets   = %d",m_event->subbands.beamlets().extent(firstDim)));
    LOG_DEBUG(formatString("second dim beamlets  = %d",m_event->subbands.beamlets().extent(secondDim)));
    LOG_DEBUG(formatString("thirth dim beamlets  = %d",m_event->subbands.beamlets().extent(thirdDim)));
    return (false);

}
