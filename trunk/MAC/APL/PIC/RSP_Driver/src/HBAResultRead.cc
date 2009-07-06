//#  HBAResultRead.cc: implementation of the HBAResultRead class
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
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "HBAProtocolWrite.h"
#include "HBAResultRead.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;

HBAResultRead::HBAResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

HBAResultRead::~HBAResultRead()
{
  /* TODO: delete event? */
}

void HBAResultRead::sendrequest()
{
	uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();
	//LOG_DEBUG_STR("HBA request result for " << (int) global_blp);

	// skip update if the RCU settings have not been applied yet
	if (RTC::RegisterState::READ != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL) &&
		RTC::RegisterState::READ != Cache::getInstance().getState().hbaprotocol().get(global_blp * MEPHeader::N_POL + 1)) {
		setContinue(true);
		return;
	}

	// set appropriate header
	EPAReadEvent rcuresult;
	rcuresult.hdr.set(MEPHeader::RCU_RESULTY_HDR, 1 << getCurrentIndex(),
					  MEPHeader::READ, sizeof(HBAProtocolWrite::i2c_result));

	m_hdr = rcuresult.hdr; // remember header to match with ack
	getBoardPort().send(rcuresult);
}

void HBAResultRead::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult HBAResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	if (EPA_RCU_RESULT != event.signal) {
		LOG_WARN("HBAResultRead::handleack:: unexpected ack");
		return GCFEvent::NOT_HANDLED;
	}
  
	EPARcuResultEvent ack(event);
	uint8 global_blp = (getBoardId() * StationSettings::instance()->nrBlpsPerBoard()) + getCurrentIndex();

#if 0
	string received, expected;
	hexdump (received, (char*)(&(ack.result)), HBAProtocolWrite::RESULT_SIZE);
	hexdump (expected, HBAProtocolWrite::i2c_result, HBAProtocolWrite::RESULT_SIZE);
	LOG_INFO_STR("ack for hba[" << (int)(global_blp) << "]=" << received);
	LOG_INFO_STR("exp for hba[" << (int)(global_blp) << "]=" << expected);
#endif

	// copy read-values to HBAReadings-cache.
	string	faultyElements;
	uint8* cur = ack.result + HBAProtocolWrite::RESULT_DELAY_OFFSET;
	for (int element = 0; element < MEPHeader::N_HBA_DELAYS; element++) {
		if ((int)(*(cur-2)) != (int)(0x81+element)) {
			faultyElements.append(formatString("%d,", element));
			Cache::getInstance().getBack().getHBAReadings()()((global_blp*2),   element) = 255;	// X
			Cache::getInstance().getBack().getHBAReadings()()((global_blp*2)+1, element) = 255;	// Y
		}
		else {
			Cache::getInstance().getBack().getHBAReadings()()((global_blp*2),   element) = *(cur+0);	// X
			Cache::getInstance().getBack().getHBAReadings()()((global_blp*2)+1, element) = *(cur+1);	// Y
		}
		cur += HBAProtocolWrite::RESULT_DELAY_STRIDE;
	}
	// Copy result for other cache also.
	Cache::getInstance().getFront().getHBAReadings() = Cache::getInstance().getBack().getHBAReadings();

	// check result
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("HBAResultRead::handleack: invalid ack");
		Cache::getInstance().getState().hbaprotocol().read_error(global_blp * MEPHeader::N_POL);
		Cache::getInstance().getState().hbaprotocol().read_error(global_blp * MEPHeader::N_POL + 1);
		return GCFEvent::HANDLED;
	}

	// compare result with expected result
	if (memcmp(HBAProtocolWrite::i2c_result, ack.result, sizeof(HBAProtocolWrite::i2c_result)) == 0) {
		Cache::getInstance().getState().hbaprotocol().read_ack(global_blp * MEPHeader::N_POL);
		Cache::getInstance().getState().hbaprotocol().read_ack(global_blp * MEPHeader::N_POL + 1);
	} else {
		LOG_WARN_STR("HBAResultRead: unexpected I2C result response for element(s):" << faultyElements << 
					 " of antenna " << (int)(global_blp));
		Cache::getInstance().getState().hbaprotocol().read_error(global_blp * MEPHeader::N_POL);
		Cache::getInstance().getState().hbaprotocol().read_error(global_blp * MEPHeader::N_POL + 1);
	}

	return GCFEvent::HANDLED;
}
