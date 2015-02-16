//#  BWWrite.cc: implementation of the BWWrite class
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
#include <Common/hexdump.h>

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include <unistd.h>
#include <string.h>
#include <blitz/array.h>

#include "BWWrite.h"
#include "Cache.h"
#include "StationSettings.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

BWWrite::BWWrite(GCFPortInterface& board_port, int board_id, int blp, int regid)
	: SyncAction(board_port, board_id, MEPHeader::BF_N_FRAGMENTS*MAX_NR_BM_BANKS),
	  m_blp(blp), m_regid(regid), itsBank(0), itsActiveBanks(1), m_remaining(0), m_offset(0)
{
	
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

BWWrite::~BWWrite()
{
}

void BWWrite::sendrequest()
{
    itsActiveBanks = (MAX_BITS_PER_SAMPLE / Cache::getInstance().getBack().getBitsPerSample());
    uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + m_blp;
    if (getCurrentIndex() >= (itsActiveBanks*MEPHeader::BF_N_FRAGMENTS)) {
        if (getCurrentIndex() == (MEPHeader::BF_N_FRAGMENTS*MAX_NR_BM_BANKS - 1)) {
            Cache::getInstance().getState().bf().write_ack(global_blp * MEPHeader::N_PHASEPOL + m_regid);
        }
        setContinue(true);
        return;
    }
    
    
    itsBank = (getCurrentIndex() / MEPHeader::BF_N_FRAGMENTS) ;
        
	// no conditional, update every second

	// reset m_offset and m_remaining for each register
	if (0 == (getCurrentIndex()%MEPHeader::BF_N_FRAGMENTS)) {
		m_offset = MEPHeader::N_LOCAL_XLETS * MEPHeader::WEIGHT_SIZE;
		m_remaining = MEPHeader::BF_XROUT_SIZE - m_offset; // representative for XR, XI, YR, YI size
	}

	if (m_regid < MEPHeader::BF_XROUT || m_regid > MEPHeader::BF_YIOUT) {
		LOG_FATAL("invalid regid");
		exit(EXIT_FAILURE);
	}

	// send next BF configure message
	EPABfCoefsWriteEvent bfcoefs;

	size_t size = (MEPHeader::N_BEAMLETS * MEPHeader::WEIGHT_SIZE) / MEPHeader::BF_N_FRAGMENTS;
	LOG_DEBUG_STR("size=" << size);

	// this code is only guaranteed to work under the following conditions
	ASSERT(size < MEPHeader::FRAGMENT_SIZE);
	
	switch (m_regid) {
	case MEPHeader::BF_XROUT:
		bfcoefs.hdr.set( MEPHeader::WRITE, 
                         1 << m_blp,
                         MEPHeader::BF,
                         MEPHeader::BF_XROUT+(itsBank*4),
                         size,
                         m_offset);
		break;
	case MEPHeader::BF_XIOUT:
	    bfcoefs.hdr.set( MEPHeader::WRITE, 
                         1 << m_blp,
                         MEPHeader::BF,
                         MEPHeader::BF_XIOUT+(itsBank*4),
                         size,
                         m_offset);
		break;
	case MEPHeader::BF_YROUT:
	    bfcoefs.hdr.set( MEPHeader::WRITE, 
                         1 << m_blp,
                         MEPHeader::BF,
                         MEPHeader::BF_YROUT+(itsBank*4),
                         size,
                         m_offset);
		break;
	case MEPHeader::BF_YIOUT:
	    bfcoefs.hdr.set( MEPHeader::WRITE, 
                         1 << m_blp,
                         MEPHeader::BF,
                         MEPHeader::BF_YIOUT+(itsBank*4),
                         size,
                         m_offset);
		break;
	}

#if 0	
	if (getBoardId() == 11) {
		LOG_INFO_STR("BWWrite:board:" << getBoardId() << ",global_blp=" << (int)global_blp << ",bank=" << itsBank 
			<< "," << bfcoefs.hdr);
	}
#endif

	// create blitz view om the weights in the bfcoefs message to be sent to the RSP hardware
	int nbeamlets_per_fragment = MEPHeader::N_BEAMLETS / MEPHeader::BF_N_FRAGMENTS;
	Array<complex<int16>, 2> weights(nbeamlets_per_fragment, N_POL);
	bfcoefs.coef.setBuffer(weights.data(), weights.size() * sizeof(complex<uint16>));

#if 0
	Array<int, 2> index(MEPHeader::N_BEAMLETS, N_POL);
	Array<int, 2> mapped_index(nbeamlets_per_fragment, N_POL);

	for (int beamlet = 0; beamlet < MEPHeader::N_BEAMLETS; beamlet++) {
		for (int pol = 0; pol < N_POL; pol++) {
			index(beamlet, pol) = beamlet * N_POL + pol;
		}
	}
	mapped_index = 0;
#endif

	LOG_DEBUG_STR("weights shape=" << weights.shape());

	ASSERT(MEPHeader::N_BEAMLETS % MEPHeader::BF_N_FRAGMENTS == 0);
	for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
		int hw_offset = lane;
		int cache_offset = lane * (MEPHeader::N_BEAMLETS / MEPHeader::N_SERDES_LANES) + ((getCurrentIndex() % MEPHeader::BF_N_FRAGMENTS) * nbeamlets_per_fragment / MEPHeader::N_SERDES_LANES);

		Range hw_range(hw_offset, hw_offset + nbeamlets_per_fragment - MEPHeader::N_BLPS, MEPHeader::N_BLPS);
		Range cache_range(cache_offset, cache_offset + (nbeamlets_per_fragment / MEPHeader::N_SERDES_LANES) - 1, 1);
#if 0		
		if (getBoardId() == 11) {
			LOG_INFO_STR("board=" << getBoardId() << ",bank=" << itsBank << ",lane=" << lane 
					<< (m_regid / 2 ? ",Y" : ",X") << (m_regid % 2 ? "I" : "R")
					<< ", hw_range=" << hw_range << ", cache_range=" << cache_range);
		}
#endif
		// X = normal 0
		weights(hw_range, 0) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2, itsBank, cache_range);

		// Y = normal 1
		weights(hw_range, 1) = Cache::getInstance().getBack().getBeamletWeights()()(0, global_blp * 2 + 1, itsBank, cache_range);

#if 0
			mapped_index(hw_range, 0) = index(cache_range, 0);
			mapped_index(hw_range, 1) = index(cache_range, 1);
#endif
	}

#if 0
		LOG_DEBUG_STR("mapped_index=" << mapped_index);
#endif

	// update m_remaining and m_offset for next write
	m_remaining -= size;
	m_offset    += size;

	//
	// conjugate weights to get the correct matrix

	// weight a_r + i . a_i
	//
	// (a_r + i . a_i) . (x_r + i . x_i) = ( a_r . x_r - a_i . x_i ) + i ( a_i . x_r + a_r . x_i )
	//
	// This is equal to:
	//
	// ( x_out_r )   ( a_r  -a_i )   ( x_in_r )
	// (         ) = (           ) * (        )
	// ( x_out_i )   ( a_i   a_r )   ( x_in_i )
	//
	// and the same for y
	//
	// To send to correct weights to the beamformer we must therefore
	// take the conjugate of the weight (a_r, a_i) to produce (a_r, -a_i)
	// for the x_our_r component and take the conjugate AND multiply by i
	// the weight (a_r, a_i) to produce (a_i, a_r).
	//

	// swap xpol and ypol if isSwappedXY() is true
	int xpol = 0;
	int ypol = 1;
	LOG_DEBUG_STR("XY= straight");
	if (true == Cache::getInstance().getBack().isSwappedXY(global_blp)) {
		xpol = 1;
		ypol = 0;
		LOG_DEBUG_STR("XY= swapped");
	}
    
	weights = conj(weights);

	switch (m_regid) {
		case MEPHeader::BF_XROUT: {
			// weights for x-real part
			// no added conversions needed

			// y weights should be 0
			weights(Range::all(), ypol) = 0;
		}
		break;

		case MEPHeader::BF_XIOUT: {
			// multiply by i to get correct weights
			// weights for x-imaginary part
			weights *= complex<int16>(0,1);

			// y weights should be 0
			weights(Range::all(), ypol) = 0;
		}
		break;

		case MEPHeader::BF_YROUT: {
			// weights for y-real part
			// no added conversions needed

			// x weights should be 0
			weights(Range::all(), xpol) = 0;
		}
		break;

		case MEPHeader::BF_YIOUT: {
			// multiply by i to get correct weights
			// weights for y-imaginary part
			weights *= complex<int16>(0,1);

			// x weights should be 0
			weights(Range::all(), xpol) = 0;
		}
		break;
	}// switch

#if 0
	// Debugging the waits send to one BLP by hexdumping the data before setting it on ethernet.
	if (global_blp == 32 && m_offset < 1000) {
		string	hd;
		hexdump (hd, weights.data(), weights.size() * sizeof(complex<uint16>));
		LOG_INFO_STR("global_blp=32, regid="<< m_regid << ",data=" << hd << endl);
	}
#endif

	m_hdr = bfcoefs.hdr;
	getBoardPort().send(bfcoefs);
}

void BWWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult BWWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal) {
		LOG_INFO_STR(formatString("event.signal=%d", event.signal)); 
		LOG_WARN("BWWrite::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);

	uint8 global_blp = (getBoardId() * NR_BLPS_PER_RSPBOARD) + m_blp;

	if (!ack.hdr.isValidAck(m_hdr)) {
		Cache::getInstance().getState().bf().write_error(global_blp * MEPHeader::N_PHASEPOL + m_regid);

		LOG_ERROR("BWWrite::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;

	} else {

		//
		// Last fragment signals completion
		//
		if ((MEPHeader::BF_N_FRAGMENTS*itsActiveBanks-1) == getCurrentIndex()) {
		  Cache::getInstance().getState().bf().write_ack(global_blp * MEPHeader::N_PHASEPOL + m_regid);
		}
	}
	return GCFEvent::HANDLED;
}


