//#  RawBlockWrite.cc: implementation of the RawBlockWrite class
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
#include "RawBlockWrite.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;
using namespace blitz;

//
// RawBlockWrite(port, boardID)
//
RawBlockWrite::RawBlockWrite(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, 1)
{
	memset(&m_hdr, 0, sizeof(MEPHeader));
}

//
// ~RawBlockWrite()
//
RawBlockWrite::~RawBlockWrite()
{
  /* TODO: delete event? */
}

//
// sendrequest()
//
void RawBlockWrite::sendrequest()
{
	// skip update if nothing was modified
	if (Cache::getInstance().getState().rawdatawrite().get(getBoardId()) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().rawdatawrite().unmodified(getBoardId());
		setContinue(true);
//		LOG_INFO_STR("@@@RawBlockWrite::sendrequest(" << getBoardId() << "):false");
		return;
	}
//	LOG_INFO_STR("@@@RawBlockWrite::sendrequest(" << getBoardId() << "):true");

	// Cache was modified, construct an EPA message and send it.
	EPAWriteEvent	writeEvent;
	RawDataBlock_t&	rdb = Cache::getInstance().getBack().getRawDataBlock();	// pointer to info
	// construct header, but overwrite complete address with address from user
	writeEvent.hdr.set(MEPHeader::RSP_RAWDATA_WRITE, 0, MEPHeader::WRITE, rdb.dataLen, rdb.offset);
	memcpy((void*)&(writeEvent.hdr.m_fields.addr), (void*)&(rdb.address), sizeof(rdb.address));
	// construct data part
	writeEvent.payload.setBuffer(rdb.data, rdb.dataLen);

	m_hdr = writeEvent.hdr; // remember header to match with ack

//	string	hDump;	// DEBUG
//	hexdump(hDump, (void*)&m_hdr.m_fields, MEPHeader::SIZE); // DEBUG
//	LOG_INFO (hDump);
//	hDump.clear();
//	hexdump(hDump, writeEvent.payload.getBuffer(), writeEvent.payload.getDataLen());
//	LOG_INFO (hDump);

	// finally send the message
	getBoardPort().send(writeEvent);

	// we never want a rawBlock command to be repeated so mark it 'done' on forehand.
	Cache::getInstance().getState().rawdatawrite().write_ack(getBoardId());
}

//
// sendrequest_status()
//
void RawBlockWrite::sendrequest_status()
{
	// intentionally left empty
}

//
// handleack(event, port)
//
GCFEvent::TResult RawBlockWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
//	LOG_INFO_STR("@@@RawBlockWrite::handleack(" << getBoardId() << ")");

  	// Note: we don't have to check the event.signal here because it is always safe to 
	//		 convert an event to a generic readackEvent.
	//		 We can't even check it because the user made up the address and therefor the response-type.
	EPAWriteackEvent ack(event);

//	string	hDump;		/// DEBUG
//	hexdump(hDump, (void*)&(ack.hdr.m_fields), MEPHeader::SIZE);
//	LOG_INFO (hDump);

	// check result
	if (!ack.hdr.isValidAck(m_hdr)) {
		LOG_ERROR("RawBlockWrite::handleack: invalid ack");
		Cache::getInstance().getState().rawdatawrite().write_error(getBoardId());
		return GCFEvent::HANDLED;
	}

	// Mark command ok.
	Cache::getInstance().getState().rawdatawrite().write_ack(getBoardId());

	return GCFEvent::HANDLED;
}
