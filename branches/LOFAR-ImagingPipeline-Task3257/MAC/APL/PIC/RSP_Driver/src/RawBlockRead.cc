//#  RawBlockRead.cc: implementation of the RawBlockRead class
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
#include "RawBlockRead.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;

//
// RawBlockRead(port, boardID)
//
RawBlockRead::RawBlockRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

//
// ~RawBlockRead()
//
RawBlockRead::~RawBlockRead()
{
  /* TODO: delete event? */
}

//
// sendrequest()
//
void RawBlockRead::sendrequest()
{
	// skip update if nothing was modified
	if (Cache::getInstance().getState().rawdataread().get(getBoardId()) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().rawdataread().unmodified(getBoardId());
		setContinue(true);
//		LOG_INFO_STR("@@@RawBlockRead::sendrequest(" << getBoardId() << "):false");
		return;
	}
//	LOG_INFO_STR("@@@RawBlockRead::sendrequest(" << getBoardId() << "):true");

	// Cache was modified, construct an EPA message and send it.
	// Note that the buffer in the cache already contains the whole message, just copy the bytes
	// to the right places in the message.
	EPAReadEvent	rawData;
	RawDataBlock_t&	rdb = Cache::getInstance().getBack().getRawDataBlock();
	rawData.hdr.set(MEPHeader::RSP_RAWDATA_READ, 0, MEPHeader::READ, rdb.dataLen, rdb.offset);
	// overwrite complete address
	memcpy((void*)&(rawData.hdr.m_fields.addr), (void*)&(rdb.address), sizeof(rdb.address));
	m_hdr = rawData.hdr; // remember header to match with ack

//	string	hDump;	// DEBUG
//	hexdump(hDump, (void*)&m_hdr.m_fields, MEPHeader::SIZE); // DEBUG
//	LOG_INFO_STR ("RAWBLOCKREAD: "  << hDump);

	// finally send the message
	getBoardPort().send(rawData);
}

//
// sendrequest_status()
//
void RawBlockRead::sendrequest_status()
{
	// intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult RawBlockRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
//	LOG_INFO_STR("@@@RawBlockRead::handleack(" << getBoardId() << ")");

  	// Note: we don't have to check the event.signal here because it is always safe to 
	//		 convert an event to a generic readackEvent.
	//		 We can't even check it because the user made up the address and there for the response-type.
	EPAReadackEvent ack(event);

//	string	hDump;		/// DEBUG
//	hexdump(hDump, (void*)&(ack.hdr.m_fields), MEPHeader::SIZE);
//	LOG_INFO_STR ("RAWBLOCKREAD REPLY: " << hDump);

	// check result
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("RawBlockRead::handleack: invalid ack");
		Cache::getInstance().getState().rawdataread().write_error(getBoardId());
		return GCFEvent::HANDLED;
	}

	// Mark command ok.
	Cache::getInstance().getState().rawdataread().write_ack(getBoardId());

	// copy the stuff
	RawDataBlock_t&		rdb = Cache::getInstance().getBack().getRawDataBlock();
	rdb.dataLen = ack.hdr.m_fields.payload_length;
	memcpy(rdb.data, ack.data, rdb.dataLen);

//	hDump.clear();		/// DEBUG
//	hexdump(hDump, (void*)&(rdb.data), rdb.dataLen);
//	LOG_INFO (hDump);

	return GCFEvent::HANDLED;
}
