//#  SerdesWrite.cc: implementation of the SerdesWrite class
//#
//#  Copyright (C) 2009
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

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RTCCommon/PSAccess.h>
#include <blitz/array.h>

#include "StationSettings.h"
#include "SerdesWrite.h"
#include "Cache.h"

#define N_RETRIES 3

using namespace blitz;
using namespace LOFAR;
using namespace RSP;
using namespace RTC;

SerdesWrite::SerdesWrite(GCFPortInterface& board_port, int board_id, int nrCycles)
  : SyncAction(board_port, board_id, nrCycles)
{
	memset(&itsHeader, 0, sizeof(MEPHeader));
}

SerdesWrite::~SerdesWrite()
{
  /* TODO: delete event? */
}

void SerdesWrite::sendrequest()
{
	//LOG_INFO(formatString(">>>> SerdesWrite:board=%d,index=%d", getBoardId(), getCurrentIndex()));

	// If Cache was not written, we are ready
	if (Cache::getInstance().getState().sbwState().get(getBoardId()) != RTC::RegisterState::WRITE) {
		Cache::getInstance().getState().sbwState().unmodified(getBoardId());
		setContinue(true);
		setFinished();
		return;
	}
	//LOG_INFO(">>>> SerdesWrite:Write actions active");

	SerdesBuffer&	sdBuf = Cache::getInstance().getBack().getSdsWriteBuffer();
	int				dataOffset  = getCurrentIndex()-1;	// place in the SerdesBuffer where the data is
	int				sendDataMsg = getCurrentIndex()%2;	// send MDIO data or MDIO header message
	int				sendWriteHdr= getCurrentIndex()%4;	// send 'address' header or 'write' header

	// are we already beyond the data?
	if (getCurrentIndex() >= sdBuf.getDataLen()) {		// reached EOdata?
		Cache::getInstance().getState().sbwState().write_ack(getBoardId());	// mark cache ready
		//LOG_INFO(">>>> SerdesWrite:Beyond data");
		setContinue(true);
		setFinished();
		return;
	}

	// still data left, send it as HEADER or DATA message.
	if (!sendDataMsg) {	
		EPAMdioHeaderEvent	event;
		event.hdr.set(MEPHeader::SERDES_HEADER_HDR);
		event.header[0] = 0x16;
		event.header[1] = sendWriteHdr ? 0x10 : 0x00;
		itsHeader = event.hdr;
		//LOG_INFO(formatString(">>>> SerdesWrite:Write header:%02x %02x", event.header[0], event.header[1]));
		getBoardPort().send(event);
	}
	else {
		EPAMdioDataEvent	event;
		event.hdr.set(MEPHeader::SERDES_DATA_HDR);
		memcpy(&event.data[0], sdBuf.getBufferPtr() + dataOffset, MEPHeader::MDIO_DATA_SIZE);
		itsHeader = event.hdr;
		//LOG_INFO(formatString(">>>> SerdesWrite:Write data:%02x %02x", event.data[0], event.data[1]));
		getBoardPort().send(event);
	}
}

void SerdesWrite::sendrequest_status()
{
	// intentionally left empty
}

GCFEvent::TResult SerdesWrite::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
	//LOG_INFO("SerdesWrite::handleack");
	if (event.signal != EPA_WRITEACK) {
		LOG_WARN("SerdesWrite::handleack: unexpected ack");
		return (GCFEvent::NOT_HANDLED);		// stop msg stream to this RSPboard
	}

	EPAWriteackEvent ack(event);
	if (!ack.hdr.isValidAck(itsHeader)) {
		Cache::getInstance().getState().sbwState().write_error(getBoardId());
		LOG_ERROR("SerdesWrite::handleack: invalid ack");
		return GCFEvent::NOT_HANDLED;
	}

	return GCFEvent::HANDLED;
}
