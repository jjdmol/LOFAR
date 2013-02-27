//#  BstRead.cc: implementation of the BstRead class
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
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/Statistics.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "BstRead.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

BstRead::BstRead(GCFPortInterface& board_port, int board_id, int lane_id)
  : SyncAction(board_port, board_id, MEPHeader::N_SERDES_LANES)  // MEPHeader::MAX_N_BANKS in new firmware
{
	itsLaneId = lane_id;
	itsRealLaneId = (lane_id < 10)?lane_id:(lane_id-10);
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

BstRead::~BstRead()
{
}

void BstRead::sendrequest()
{
	if ((itsLaneId >= 10) && !Cache::getInstance().getBack().isSplitterActive()) {
		setContinue(true);
		return;
	}

	if ((( Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_maj * 10) +
		Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_min     ) < 74) {
		// if old firmware version do
		if (getCurrentIndex() != itsRealLaneId) {
			setContinue(true);
			return;
		}
	}
	else {
		// if new firmware version do
		if (getCurrentIndex() >= (MAX_BITS_PER_SAMPLE / Cache::getInstance().getBack().getBitsPerSample())) {
			setContinue(true);
			return;
		}
	}

	EPAReadEvent bstread;

	bstread.hdr.set(MEPHeader::READ, MEPHeader::DST_RSP, MEPHeader::BST, getCurrentIndex(), MEPHeader::BST_POWER_SIZE); 
	m_hdr = bstread.hdr;
	getBoardPort().send(bstread);

#if 0
string s;
hexdump(s, (void*)&bstread, sizeof(bstread));
LOG_INFO_STR("BSTREADREQUEST=" << s);
LOG_INFO(formatString("BSTREAD:board=%d,dstid=%d,pid=%d,regid=%d,offset=%d,payload=%d,seqnr=%d", getBoardId(), bstread.hdr.m_fields.addr.dstid, bstread.hdr.m_fields.addr.pid, bstread.hdr.m_fields.addr.regid, bstread.hdr.m_fields.offset, bstread.hdr.m_fields.payload_length, bstread.hdr.m_fields.seqnr));

if (getBoardId() == 11) {
    LOG_INFO(formatString("BSTREAD:board=%d,dstid=%d,pid=%d,regid=%d,offset=%d,payload=%d,seqnr=%d", getBoardId(), bstread.hdr.m_fields.addr.dstid, bstread.hdr.m_fields.addr.pid, bstread.hdr.m_fields.addr.regid, bstread.hdr.m_fields.offset, bstread.hdr.m_fields.payload_length, bstread.hdr.m_fields.seqnr));
}
#endif
}

void BstRead::sendrequest_status()
{
	// intentionally left empty
}

/**
 * Function to convert the semi-floating point representation used by the
 * EPA firmware to a double.
 */
inline double convert_uint32_to_double(uint32 val)
{
	int64 val64;
	// check if extent bit(bit31) is high
	if (val & (1 << 31)) {
		// if extent = high, multiply mantissa(bit30..0) by 2^23
		val64 = (int64)(val & ~(1 << 31)) << 23;
	}
	else {
		val64 = (int64)val;
	} 

/*
	uint32 e = val & (1<<31);
	uint32 s = val & (1<<30);
	int32  m = val & ((1<<30)-1); 

	if (s) m = m - (1<<30);
	if (e) {
		val64 = (int64)m << 23);
	} else {
		val64 = m;
	}
*/
	return (double)(val64);
}
BZ_DECLARE_FUNCTION_RET(convert_uint32_to_double, double)

GCFEvent::TResult BstRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_BST_STATS != event.signal) {
		LOG_WARN("BstRead::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPABstStatsEvent ack(event);

	if (!ack.hdr.isValidAck(m_hdr)) {
		Cache::getInstance().getState().bst().read_error(itsRealLaneId);
		LOG_ERROR("BstRead::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	LOG_DEBUG(formatString("BstRead::handleack: boardid=%d lane=%d", getBoardId(), itsLaneId));

	int swstart;
	// In new firmware versions BST registers are used in a different way
	if ((( Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_maj * 10) +
		Cache::getInstance().getBack().getVersions().bp()(getBoardId()).fpga_min     ) < 74) {
		swstart = (itsRealLaneId * MEPHeader::N_BEAMLETS);
	}
	else {          
		swstart = (itsRealLaneId * MEPHeader::N_BEAMLETS) + (getCurrentIndex() * MEPHeader::N_DATA_SLOTS);
	}

	Range fragment_range(swstart, swstart+MEPHeader::N_DATA_SLOTS-1);

	// normal set 0/1, if splitter active also 2/3
	int beamletsSet = (itsLaneId < 10)?0:2;
  
	LOG_DEBUG_STR("fragment_range[" << getBoardId() << "," << getCurrentIndex() << "]=" << fragment_range);

	if (getCurrentIndex() != ack.hdr.m_fields.addr.regid) {
		LOG_ERROR("invalid bst ack");
		return GCFEvent::HANDLED;
	}

	Array<uint32, 2> stats((uint32*)&ack.stat,
	shape((MEPHeader::BST_POWER_SIZE / sizeof(uint32)) / N_POL, N_POL), neverDeleteData);
	LOG_DEBUG(formatString("real_lane=%d, beamletsSet=%d, swstart=%d", itsRealLaneId, beamletsSet, swstart));
	LOG_DEBUG_STR("stats:" << stats); 
	Array<double, 2>& cache(Cache::getInstance().getBack().getBeamletStats()());

	// x-pol beamlet statistics: copy and convert to double
	cache(beamletsSet,     fragment_range) =
	    convert_uint32_to_double(stats(Range::all(), 0));

	// y-pol beamlet statistics: copy and convert to double
	cache(beamletsSet + 1, fragment_range) =
	    convert_uint32_to_double(stats(Range::all(), 1));

	Cache::getInstance().getState().bst().read_ack(itsRealLaneId);

	return GCFEvent::HANDLED;
}
