//#  CRSyncWrite.cc: implementation of the CRSyncWrite class
//#
//#  Copyright (C) 2011
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
//#  $Id: CRSyncWrite.cc 17606 2011-03-22 12:49:57Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>

#include "StationSettings.h"
#include "CRSyncWrite.h"
#include "Cache.h"

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RTC;

CRSyncWrite::CRSyncWrite(GCFPortInterface& board_port, int board_id, blitz::Array<int, 1> delaySteps)
  : SyncAction(board_port, board_id, 1+max(delaySteps))
{
	ASSERTSTR(delaySteps.extent(firstDim) == NR_BLPS_PER_RSPBOARD, "Expected " << NR_BLPS_PER_RSPBOARD << 
				" delay-steps not " << delaySteps.extent(firstDim));

	itsDelays.resize(delaySteps.shape());
	itsCounters.resize(delaySteps.shape());
	itsDelays   = delaySteps;
	itsCounters = itsDelays;

	memset(&m_hdr, 0, sizeof(MEPHeader));
	doAtInit();								// only during initialisation mode
}

CRSyncWrite::~CRSyncWrite()
{
	/* TODO: delete event? */
}

void CRSyncWrite::sendrequest()
{
	// No actions requested? done.
	if ((Cache::getInstance().getState().crcontrol().get(getBoardId()) != RTC::RegisterState::WRITE) &&
	    (Cache::getInstance().getState().crcontrol().get(getBoardId()) != RTC::RegisterState::READ)) {
		Cache::getInstance().getState().crcontrol().unmodified(getBoardId());
		setContinue(true);
		return;
	}

	// prepare and send command
	uint16	blpid(0);
	EPACrControlEvent CRSyncEvent;
	if (Cache::getInstance().getState().crcontrol().get(getBoardId()) == RTC::RegisterState::READ) {
		// start of cycle, log unfinished cycles
		for (int b = 0; b < NR_BLPS_PER_RSPBOARD; b++) {
			if (itsCounters(b) && itsCounters(b) != itsDelays(b)) {
				LOG_WARN(formatString("PPS delay[%d][%]: %d out of %d written", 
							getBoardId(), b, itsDelays(b)-itsCounters(b), itsDelays(b)));
			}
		}
		CRSyncEvent.hdr.set(MEPHeader::CR_SYNCDELAY_HDR, MEPHeader::DST_ALL_BLPS);	// to all blps
		CRSyncEvent.control = 0x00;											// reset pps delay
		itsCounters = itsDelays;
	    Cache::getInstance().getState().crcontrol().write(getBoardId());	// change to write cycle next time
		LOG_INFO(formatString("PPS: setting register[%d] to write", getBoardId()));
	}
	else {
		uint16	mask(1);
		for (int b = 0; b < NR_BLPS_PER_RSPBOARD; b++, mask*=2) {
			if (itsCounters(b)) {	// does this blp need an extra delay steps? add it to the mask
				blpid |= mask;
				itsCounters(b)--;
			}
		}	
		CRSyncEvent.hdr.set(MEPHeader::CR_SYNCDELAY_HDR, blpid);
		CRSyncEvent.control = 0x01;  // SyncEdge=rise, SyncDelay=Increment
	}

//	if (getBoardId() == 0) {
		LOG_INFO(formatString("PPS delay board %d: 0x%02x (%d %d %d %d)", getBoardId(), blpid, itsCounters(0), itsCounters(1), itsCounters(2), itsCounters(3)));
//	}
	m_hdr = CRSyncEvent.hdr;
	getBoardPort().send(CRSyncEvent);
}

void CRSyncWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult CRSyncWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (event.signal != EPA_WRITEACK) {
		LOG_WARN("CRSyncWrite::handleack: unexpected ack");
		return (GCFEvent::NOT_HANDLED);
	}

	EPAWriteackEvent ack(event);

	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("CRSyncWrite::handleack: invalid ack");
		Cache::getInstance().getState().crcontrol().write_error(getBoardId());
		LOG_INFO(formatString("PPS: setting register[%d] to writeERROR", getBoardId()));
		itsCounters = itsDelays;
		return (GCFEvent::NOT_HANDLED);
	}

	// are we done?
	if (sum(itsCounters) == 0) {
		// change state to indicate that it has been applied in the hardware
		Cache::getInstance().getState().crcontrol().write_ack(getBoardId());
		LOG_INFO(formatString("PPS: setting register[%d] to writeACK", getBoardId()));
	}
	return (GCFEvent::HANDLED);
}
