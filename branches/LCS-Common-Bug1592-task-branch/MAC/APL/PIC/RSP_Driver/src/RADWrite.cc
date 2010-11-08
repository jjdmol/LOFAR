//#  RADWrite.cc: implementation of the RADWrite class
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

#include <StationSettings.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>

#include "RADWrite.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

RADWrite::RADWrite(GCFPortInterface& board_port, int board_id)
	: SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
	doAtInit();
}

RADWrite::~RADWrite()
{
	/* TODO: delete event? */
}

void RADWrite::sendrequest()
{
	// skip update if the RAD does not need to be written
	if (RTC::RegisterState::WRITE != Cache::getInstance().getState().rad().get(getBoardId())) {
		Cache::getInstance().getState().rad().unmodified(getBoardId());
		setContinue(true);
		return;
	}

	EPARadSetEvent rad;

	rad.hdr.set(MEPHeader::RAD_BP_HDR);
	rad.lanemode = 0;

	/*
	 * lane mode: one byte for each lane
	 * format: XXXXAABB
	 *
	 * where XX = don't care
	 *       AA = xlet mode
	 *       BB = blet mode
	 *
	 * mode 0b00 = ignore remote data (only local)  DEFAULT
	 * mode 0b01 = disable
	 * mode 0b10 = combine local and remote data
	 * mode 0b11 = ignore local data (only remote)
	 */
	
	for (int lane = 0; lane < MEPHeader::N_SERDES_LANES; lane++) {
		uint8	mode 	 = 0x00; // default is to ignore remote data (first board in ring)
		int		testlane = lane;
		
		// if the splitter is active, use for the upper boards the settings of ring-1
		if (Cache::getInstance().getBack().isSplitterActive()) {
			if ((int)getBoardId() >= (StationSettings::instance()->nrRspBoards() / 2)) {
				testlane += 10; // select ring-1
			}
		} 
		
		int blet_out = GET_CONFIG(formatString("RSPDriver.LANE_%02d_BLET_OUT", testlane).c_str(), i) % StationSettings::instance()->nrRspBoards();
		int xlet_out = GET_CONFIG(formatString("RSPDriver.LANE_%02d_XLET_OUT", testlane).c_str(), i) % StationSettings::instance()->nrRspBoards();
//		LOG_INFO_STR("rad.lane[" << testlane << "], blet_out=" << blet_out << ", xlet_out=" << xlet_out << ", RSPboard=" << getBoardId());
		
		// if there are more than 1 boards and
		// if this board is not the first board in the ring
		// it should combine local and remote data
		// unless disables explicitly from the configuration file
		if (0 == GET_CONFIG("RSPDriver.IGNORE_REMOTE_DATA", i)) {
			if (StationSettings::instance()->nrRspBoards() > 1) {
				int exclude_board;
				// if the splitter in not active
				if (Cache::getInstance().getBack().isSplitterActive() == false) {
					exclude_board = (int)getBoardId() - 1;
					if (exclude_board < 0) exclude_board += StationSettings::instance()->nrRspBoards();
					exclude_board %= StationSettings::instance()->nrRspBoards();
				}
				// if the splitter is active data direction is reversed,
				// and the ring is divided in two equal rings
				else {
					exclude_board = (int)getBoardId() + 1;
					
					if ((int)getBoardId() < (StationSettings::instance()->nrRspBoards() / 2)) {
						if (exclude_board >= (StationSettings::instance()->nrRspBoards() / 2)) {
							exclude_board = 0;
						}
					}
					else {
						if (exclude_board >= StationSettings::instance()->nrRspBoards()) {
							exclude_board = (StationSettings::instance()->nrRspBoards() / 2);
						}
					}
				}
				if (exclude_board != blet_out) mode |= 0x02;
				if (exclude_board != xlet_out) mode |= 0x08;
			}
		}
		rad.lanemode |= ((uint32)mode) << (8 * lane);
	}

	LOG_INFO_STR(formatString("rad.lanemode(rspboard=%d)=0x%08x", getBoardId(), htonl(rad.lanemode)));

	m_hdr = rad.hdr;
	getBoardPort().send(rad);

}

void RADWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult RADWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_WRITEACK != event.signal)
	{
		LOG_WARN("RADWrite::handleack: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}

	EPAWriteackEvent ack(event);

	if (!ack.hdr.isValidAck(m_hdr))
	{
		LOG_ERROR("RADWrite::handleack: invalid ack");
		Cache::getInstance().getState().rad().write_error(getBoardId());
		return GCFEvent::NOT_HANDLED;
	}

	Cache::getInstance().getState().rad().write_ack(getBoardId());
	
	return GCFEvent::HANDLED;
}
